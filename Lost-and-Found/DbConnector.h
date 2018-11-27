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

	std::uint64_t checkPassword(std::string username, std::string password) const;

	std::pair<bool, std::string> registerNewUser(std::string username, std::string password);

	std::uint64_t addItem(const std::string& item_name, const std::string& item_info, const std::string & lost_location);
	std::uint64_t addNotice(std::uint64_t finder_id, std::uint64_t item_id);

	std::vector<std::tuple<std::uint64_t, std::string, std::uint16_t>> queryNotice();

	void createTable_user() const;
	void createTable_userinfo() const;
	void createTable_item() const;
	void createTable_message() const;
	void createTable_item_notice() const;
	void createTable_application() const;
	void createTable_notice_info() const;

	void initDb();

	~DbConnector();

	static const char * DbName;

private:
	std::shared_ptr<sql::ResultSet> getUser(std::string username) const;

    sql::Driver* driver_;
    sql::Connection* con;
};


#endif
