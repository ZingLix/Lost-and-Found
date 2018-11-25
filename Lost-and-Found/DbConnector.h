#ifndef LIXTALK_DBCONNECTOR
#define LIXTALK_DBCONNECTOR

#include <cppconn/resultset.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <memory>
#include <vector>

class DbConnector {
public:
	DbConnector();

	void connect(std::string username, std::string password);

	std::shared_ptr<sql::ResultSet> getUser(std::string username) const;

	std::uint64_t checkPassword(std::string username, std::string password) const;

	std::pair<bool, std::string> registerNewUser(std::string username, std::string password);

	void createUserTable() const;

	void initDb();

	~DbConnector();

	static const char * DbName;

private:
    sql::Driver* driver_;
    sql::Connection* con;
};


#endif
