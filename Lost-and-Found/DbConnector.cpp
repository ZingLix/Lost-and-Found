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
		}else {
			return 0;
		}
	}catch ( sql::SQLException& e ) {
		LOG_ERROR << "# ERR: SQLException in " << __FILE__ << "(" << __FUNCTION__ << ") on line " 
			<< __LINE__ << "# ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() 
			<< ", SQLState: " << e.getSQLState() << " )" ;
		return 0;
	}
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

void DbConnector::createUserTable() const {
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

void DbConnector::initDb() {
	std::string DatabaseName(DbName);
	std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("Create Database " + DatabaseName));
	stmt->executeUpdate();
	con->setSchema(DbName);
	createUserTable();
}

DbConnector::~DbConnector() {
	if (con != nullptr)
		delete con;
}

