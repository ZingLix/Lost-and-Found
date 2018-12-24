#ifndef LIXTALK_DBCONNECTOR
#define LIXTALK_DBCONNECTOR

#include <cppconn/resultset.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <memory>
#include <vector>
#include "Component.h"

class DbConnector {
public:
	DbConnector();

	void connect(std::string username, std::string password);

	std::uint64_t checkPassword(std::string username, std::string password) const;

	std::pair<bool, std::string> registerNewUser(std::string username, std::string password);

	std::uint64_t addItem(const std::string& item_name, const std::string& item_info, const std::string & lost_location);
	std::uint64_t addNotice(std::uint64_t finder_id, std::uint64_t item_id);

	std::vector<std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string>> queryNotice();
	std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string> queryNotice(std::uint64_t notice_id);
	std::vector<std::tuple<std::uint64_t, std::uint64_t, std::uint16_t, std::uint64_t, std::uint64_t, std::string>> queryNotice(std::string keyword);
	std::uint64_t addApplication(std::uint64_t applicant_id, std::uint64_t notice_id);

	std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint16_t, uint64_t, std::string>>
		queryApplication(std::uint64_t user_id);

	void execApplication(std::uint64_t application_id, int status);

	void withdrawNotice(std::uint64_t notice_id);

	void withdrawApplication(std::uint64_t application_id);

	item queryItem(std::uint64_t item_id);
	void modifyItem(item i);

	userinfo queryUser(std::uint64_t user_id);
	void modifyUser(userinfo i);

	std::uint64_t addMessageRecord(std::uint64_t sender_id, std::uint64_t recver_id, std::string content);
	std::vector<message> pullMessageRecord(std::uint64_t user_id);
	std::vector<message> pullMessageRecord(std::uint64_t user_id1,std::uint64_t user_id2);

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
