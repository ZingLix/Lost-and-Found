#include "DbConnector.h"
#include "LogInfo.h"

#include <memory>
#include "User.h"

const char * DbConnector::DbName = "Lost_and_Found";
const int pullMsgCountLimit = 200;

DbConnector::DbConnector() : con(nullptr) {
	driver_ = get_driver_instance();
}

void DbConnector::connect(std::string username, std::string password) {
	con = driver_->connect("tcp://127.0.0.1:3306", username, password);
	try {
		con->setSchema(DbName);
	} catch (sql::SQLException& e) {
		if (e.getErrorCode() == 1049) {
			LOG_ERROR << "Database not existed.";
			initDb();
			LOG_INFO << "Database create success.";
		} else {
			throw;
		}
	}
}

std::shared_ptr<sql::ResultSet> DbConnector::getUser(std::string username) const {
	std::shared_ptr<sql::Statement> stmt(con->createStatement());
	stmt->execute("SELECT * from user where username = '" + username + "'");
	return std::shared_ptr<sql::ResultSet>(stmt->getResultSet());
}

std::uint64_t DbConnector::checkPassword(std::string username, std::string password) const {
	auto resultSet = getUser(username);
	resultSet->next();

	try {
		if(resultSet->getString("password") == password) {
			return resultSet->getUInt64("user_id");
		}
	}catch ( sql::SQLException& e ) {
		LOG_ERROR << "# ERR: SQLException in " << __FILE__ << "(" << __FUNCTION__ << ") on line " 
			<< __LINE__ << "# ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() 
			<< ", SQLState: " << e.getSQLState() << " )" ;
	}
	return 0;
}

std::pair<bool,std::string> DbConnector::registerNewUser(std::string username, std::string password) {
	std::unique_ptr<sql::PreparedStatement> stmt ( con->prepareStatement(
		"call registerUser(?,?,@id)"));
	stmt->setString(1, username);
	stmt->setString(2, password);
	stmt->execute();
	stmt.reset(con->prepareStatement("SELECT @id user_id"));
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	if(res->getUInt64("user_id")==0) {
		return std::make_pair(false, "Duplicate username.");
	}
	return std::make_pair(true, std::string());
}

void DbConnector::createTable_user() const {
	const std::string tableName("user");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			user_id bigint Unsigned auto_increment primary key, \
			username varchar(64) not null unique,\
			password varchar(64) not null)"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("Create index idx on " + tableName + "(username)"));
	stmt->executeUpdate();
}

void DbConnector::createTable_userinfo() const {
	const std::string tableName("userinfo");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			user_id bigint Unsigned primary key, \
			email varchar(64),\
			phone varchar(64),\
			description varchar(512))"));
	stmt->executeUpdate();
}

void DbConnector::createTable_item() const {
	const std::string tableName("item");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			item_id bigint Unsigned auto_increment primary key, \
			item_name varchar(64) not null,\
			item_info varchar(512),\
			lost_location varchar(128) not null)"));
	stmt->executeUpdate();
}

void DbConnector::createTable_message() const {
	const std::string tableName("message");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			msg_seq_id bigint Unsigned auto_increment primary key, \
			sender_id bigint Unsigned not null,\
			recver_id bigint Unsigned not null,\
			content varchar(512) not null)"));
	stmt->executeUpdate();
}

void DbConnector::createTable_item_notice() const {
	const std::string tableName("item_notice");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			notice_id bigint Unsigned auto_increment primary key, \
			finder_id bigint Unsigned not null,\
			item_id bigint Unsigned not null,\
			status smallint unsigned not null default 0)"));
	stmt->executeUpdate();
}

void DbConnector::createTable_application() const {
	const std::string tableName("application");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			application_seq bigint Unsigned auto_increment primary key, \
			applicant_id bigint Unsigned not null,\
			notice_id bigint Unsigned not null,\
			status smallint unsigned not null default 0,\
			time timestamp not null default CURRENT_TIMESTAMP)"));
	stmt->executeUpdate();
}

void DbConnector::createTable_notice_info() const {
	const std::string tableName("notice_info");
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"Create Table " + tableName +
		" ( \
			notice_id bigint Unsigned primary key, \
			contact_id bigint Unsigned not null,\
			time timestamp not null DEFAULT CURRENT_TIMESTAMP)"));
	stmt->executeUpdate();
}

std::uint64_t DbConnector::addItem(const std::string& item_name, const std::string& item_info, const std::string& lost_location) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"INSERT INTO item(item_name,item_info,lost_location) VALUES (?,?,?)"));
	stmt->setString(1, item_name);
	stmt->setString(2, item_info);
	stmt->setString(3, lost_location);
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("SELECT LAST_INSERT_ID() id FROM item"));
	std::shared_ptr<sql::ResultSet> result(stmt->executeQuery());
	result->next();
	return result->getUInt64("id");
}

std::uint64_t DbConnector::addNotice(std::uint64_t finder_id, std::uint64_t item_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"INSERT INTO item_notice(finder_id,item_id) VALUES (?,?)"));
	stmt->setUInt64(1, finder_id);
	stmt->setUInt64(2, item_id);
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement("SELECT LAST_INSERT_ID() id FROM item"));
	std::shared_ptr<sql::ResultSet> result(stmt->executeQuery());
	result->next();
	std::uint64_t notice_id = result->getUInt64("id");
	stmt.reset(con->prepareStatement(
		"INSERT INTO notice_info(notice_id,contact_id) VALUES (?,?)"));
	stmt->setUInt64(1, notice_id);
	stmt->setUInt64(2, finder_id);
	stmt->executeUpdate();
	return notice_id;
}

std::vector<std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string>> DbConnector::queryNotice() {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from item_notice,notice_info where notice_info.notice_id = item_notice.notice_id"));
	std::shared_ptr<sql::ResultSet> resultset(stmt->executeQuery());
	std::vector<std::tuple<std::uint64_t,std::uint64_t, std::uint16_t, std::uint64_t,std::uint64_t,std::string>> result;
	while(resultset->next()) {
		result.push_back(std::make_tuple(resultset->getUInt64("notice_id"),resultset->getUInt64("finder_id"), resultset->getUInt("status"),
			resultset->getUInt64("item_id"),resultset->getUInt64("contact_id"),resultset->getString("time")));
	}
	return result;
}

std::vector<std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string>> DbConnector::queryNotice(
	std::string keyword) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from item_notice,notice_info,item where notice_info.notice_id = item_notice.notice_id\
		 and item.item_id = item_notice.item_id and item_name like '%" + keyword + "%'"));
	std::shared_ptr<sql::ResultSet> resultset(stmt->executeQuery());
	std::vector<std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string>> result;
	while (resultset->next()) {
		result.push_back(std::make_tuple(resultset->getUInt64("notice_id"), resultset->getUInt64("finder_id"), resultset->getUInt("status"),
			resultset->getUInt64("item_id"), resultset->getUInt64("contact_id"), resultset->getString("time")));
	}
	return result;
}

std::vector<std::uint64_t> DbConnector::queryNotice_one(
	std::uint64_t user_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from item_notice where finder_id = ?"));
	stmt->setUInt64(1, user_id);
	std::shared_ptr<sql::ResultSet> resultset(stmt->executeQuery());
	std::vector<std::uint64_t> result;
	while (resultset->next()) {
		result.push_back(resultset->getUInt64("notice_id"));
	}
	return result;
}

std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string> DbConnector::queryNotice(std::uint64_t notice_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from item_notice,notice_info where notice_info.notice_id = item_notice.notice_id\
		 and notice_info.notice_id = ?"));
	stmt->setUInt64(1, notice_id);
	std::shared_ptr<sql::ResultSet> resultset(stmt->executeQuery());
	resultset->next(); 
	return std::make_tuple(resultset->getUInt64("notice_id"), resultset->getUInt64("finder_id"), resultset->getUInt("status"),
			resultset->getUInt64("item_id"), resultset->getUInt64("contact_id"), resultset->getString("time"));
	
}

void DbConnector::initProcedure() const {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(  
		"CREATE PROCEDURE registerUser(IN username varchar(64), IN password varchar(64), OUT user_id BIGINT UNSIGNED) \
		BEGIN\
			DECLARE EXIT HANDLER FOR 1062 SET user_id = 0;\
			INSERT INTO user(username, password) VALUES(username, password);\
			SELECT LAST_INSERT_ID() into user_id;\
			INSERT INTO userinfo(user_id) VALUES(user_id);\
		END"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement(   //update notice status
		"CREATE TRIGGER trig AFTER UPDATE\
		ON item_notice FOR EACH ROW\
		BEGIN\
		IF NEW.status = 1 THEN\
			UPDATE application set status = 2 WHERE notice_id = new.notice_id and status = 0;\
		elseif new.status = 3 THEN\
			UPDATE application set status = 5 WHERE notice_id = new.notice_id and status = 0;\
		elseif new.status = 4 THEN\
			UPDATE application set status = 4 WHERE notice_id = new.notice_id and status = 0;\
		end if;\
		END"));
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement(   //add application
		"CREATE PROCEDURE addApplication(IN applicantid BIGINT UNSIGNED, IN noticeid BIGINT UNSIGNED, OUT application_id BIGINT UNSIGNED)\
		BEGIN\
		select count(*) from application where applicant_id = applicantid and notice_id = noticeid and status = 0 into application_id;\
		if application_id = 0 then\
			INSERT INTO application(applicant_id, notice_id) VALUES(applicantid, noticeid);\
			SELECT LAST_INSERT_ID() into application_id;\
		else\
			set application_id = 0;\
		end if;\
		END"));
	stmt->executeUpdate();
}

void DbConnector::initDb() {
	std::string DatabaseName(DbName);
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Database " + DatabaseName));
	stmt->executeUpdate();
	con->setSchema(DbName);
	createTable_user();
	createTable_userinfo();
	createTable_item();
	createTable_message();
	createTable_item_notice();
	createTable_application();
	createTable_notice_info();
	//initProcedure();
}

DbConnector::~DbConnector() {
	if (con != nullptr)
		delete con;
}

std::uint64_t DbConnector::addApplication(std::uint64_t applicant_id, std::uint64_t notice_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"call addApplication(?,?,@app_id)"));
	stmt->setUInt64(1, applicant_id);
	stmt->setUInt64(2, notice_id);
	stmt->execute();
	stmt.reset(con->prepareStatement("SELECT @app_id id"));
	std::shared_ptr<sql::ResultSet> result(stmt->executeQuery());
	result->next();
	return result->getUInt64("id");
}

std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint16_t, std::string>> DbConnector::queryApplication(std::uint64_t user_id) {
	std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint16_t, std::string>> res;
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select application_seq, applicant_id, application.notice_id, application.status, time \
		from application,item_notice\
		where application.notice_id = item_notice.notice_id\
		and applicant_id = "+std::to_string(user_id)));
	std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
	while(result->next()) {
		res.push_back(std::make_tuple(result->getUInt64("application_seq"), result->getUInt64("applicant_id"),
			result->getUInt64("notice_id"), result->getUInt("status"), result->getString("time")));
	}
	return res;
}

std::tuple<uint64_t, uint64_t, uint64_t, uint16_t, std::string> DbConnector::
queryApplication_one(std::uint64_t app_seq) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from application where application_seq=?"));
	stmt->setUInt64(1, app_seq);
	std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
	result->next();
	return std::make_tuple(result->getUInt64("application_seq"), result->getUInt64("applicant_id"),
		result->getUInt64("notice_id"), result->getUInt("status"), result->getString("time"));
}

void DbConnector::execApplication(std::uint64_t application_id, int status) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"update application set status = ? where application_seq = ?"));
	stmt->setInt(1, status);
	stmt->setUInt64(2, application_id);
	stmt->executeUpdate();
	if(status==1) {
		stmt.reset(con->prepareStatement(
			"select notice_id from application where application_seq = ?"));
		stmt->setUInt64(1, application_id);
		std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
		result->next();
		auto notice_id = result->getUInt64("notice_id");
		stmt.reset(con->prepareStatement(
			"update item_notice set status = 1 where notice_id = ?"));
		stmt->setUInt64(1, notice_id);
		stmt->executeUpdate();
	}
}

void DbConnector::withdrawNotice(std::uint64_t notice_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"update item_notice set status = 3 where notice_id = ?"));
	stmt->setUInt64(1, notice_id);
	stmt->executeUpdate();
}

void DbConnector::withdrawApplication(std::uint64_t application_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"update application set status = 3 where application_seq = ?"));
	stmt->setUInt64(1, application_id);
	stmt->executeUpdate();
}

item DbConnector::queryItem(std::uint64_t item_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from item where item_id = ?"));
	stmt->setUInt64(1, item_id);
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	return item(item_id, res->getString("item_name"),
		res->getString("item_info"), res->getString("lost_location"));
}

void DbConnector::modifyItem(item i) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"update item set item_name = ?, item_info = ?, lost_location = ? where item_id = ?"));
	stmt->setString(1, i.item_name);
	stmt->setString(2, i.item_info);
	stmt->setString(3, i.lost_location);
	stmt->setUInt64(4, i.item_id);
	stmt->executeUpdate();
}

userinfo DbConnector::queryUser(std::uint64_t user_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from userinfo,user where userinfo.user_id=user.user_id and userinfo.user_id = ?"));
	stmt->setUInt64(1, user_id);
	std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
	res->next();
	return userinfo(user_id, res->getString("username"), res->getString("email"),
		res->getString("phone"), res->getString("description"));
}

void DbConnector::modifyUser(userinfo i) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"update userinfo set email = ?, phone = ?, description = ? where user_id = ?"));
	stmt->setString(1, i.email);
	stmt->setString(2, i.phone);
	stmt->setString(3, i.description);
	stmt->setUInt64(4, i.user_id);
	stmt->executeUpdate();
}

std::uint64_t DbConnector::addMessageRecord(std::uint64_t sender_id, std::uint64_t recver_id, std::string content) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"insert into message(sender_id,recver_id,content) values(?,?,?)"));
	stmt->setUInt64(1, sender_id);
	stmt->setUInt64(2, recver_id);
	stmt->setString(3, content);
	stmt->executeUpdate();
	stmt.reset(con->prepareStatement(
		"select LAST_INSERT_ID() app_seq from message"));
	std::shared_ptr<sql::ResultSet> resultset(stmt->executeQuery());
	resultset->next();
	return resultset->getUInt64("app_seq");
}

std::vector<message> DbConnector::pullMessageRecord(std::uint64_t user_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from message where sender_id = ? or recver_id = ? order by msg_seq_id desc limit ?"));
	stmt->setUInt64(1, user_id);
	stmt->setUInt64(2, user_id);
	stmt->setInt(3, pullMsgCountLimit);
	std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
	std::vector<message> messages;
	while(result->next()) {
		messages.emplace_back(result->getUInt64("msg_seq_id"), result->getUInt64("sender_id"),
			result->getUInt64("recver_id"), result->getString("content"));
	}
	return messages;
}

std::vector<message> DbConnector::pullMessageRecord(std::uint64_t user_id1, std::uint64_t user_id2) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from message where (sender_id = ? and recver_id = ?) or (sender_id = ? and recver_id = ?)\
		order by msg_seq_id desc limit ?"));
	stmt->setUInt64(1, user_id1);
	stmt->setUInt64(2, user_id2);
	stmt->setUInt64(3, user_id2);
	stmt->setUInt64(4, user_id1);
	stmt->setInt(5, pullMsgCountLimit);
	std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
	std::vector<message> messages;
	while (result->next()) {
		messages.emplace_back(result->getUInt64("msg_seq_id"), result->getUInt64("sender_id"),
			result->getUInt64("recver_id"), result->getString("content"));
	}
	return messages;
}

std::vector<std::uint64_t> DbConnector::queryNotice_whoapply(std::uint64_t notice_id) {
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement(
		"select * from application where notice_id = ?"));
	stmt->setUInt64(1, notice_id);
	std::vector<std::uint64_t> app_seq_list;
	std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
	while(result->next()) {
		app_seq_list.push_back(result->getUInt64("application_seq"));
	}
	return app_seq_list;
}
