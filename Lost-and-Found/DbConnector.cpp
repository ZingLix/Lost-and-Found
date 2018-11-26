#include "DbConnector.h"
#include "LogInfo.h"

#include <memory>

const char * DbConnector::DbName = "Lost_and_Found";

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
		"INSERT INTO user(username,password) VALUES (?,?)"));
	stmt->setString(1, username);
	stmt->setString(2, password);
	try {
		stmt->executeUpdate();
	}
	catch (sql::SQLException& e) {
		if(e.getErrorCode()==1062) {
			return std::make_pair(false, "Duplicate username.");
		}
		LOG_ERROR << "# ERR: SQLException in " << __FILE__ << "(" << __FUNCTION__ << ") on line "
			<< __LINE__ << "# ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";
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
			status smallint unsigned not null)"));
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
			status smallint unsigned not null default 0)"));
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
	stmt.reset(con->prepareStatement("SELECT * FROM item"));
	std::shared_ptr<sql::ResultSet> result(stmt->executeQuery());
	result->next();
	return result->getUInt64("item_id");
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
}

DbConnector::~DbConnector() {
	if (con != nullptr)
		delete con;
}

