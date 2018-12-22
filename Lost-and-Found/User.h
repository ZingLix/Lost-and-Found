#pragma once
#include "boost/asio.hpp"
#include <memory>
#include <vector>
#include "Server.h"
#include "msg.h"
#include <boost/beast.hpp>

class User:public std::enable_shared_from_this<User>
{
public:
	using socket_ptr = std::unique_ptr<boost::asio::ip::tcp::socket>;

	User(socket_ptr&& soc_ptr,Server* s);
	virtual ~User();
	void start();
	void stop();

	socket_ptr& socket();
	std::uint64_t id();

	void on_write(const boost::system::error_code & err, size_t bytes);
	void on_read(const boost::system::error_code & err, size_t bytes);
	void do_read();
	void do_write(const std::string& str);

	void msg_exec(json_message& message);

	void user_register(json_message& message);

	void user_login(json_message& message);
	void login_success(std::uint64_t id);
	void login_failure(const std::string& reason);

	void notice_exec(json_message& message);
	void notice_post(json_message& message);
	void notice_pull(json_message& message);
	void notice_claim(json_message& message);
	void notice_apply_pull(json_message& message);
	void application_status_update(json_message& message);
	void notice_withdraw(json_message& message);
	void application_withdraw(json_message& message);
	void notice_search(json_message& message);
	void notice_query(json_message& message);
	void notice_pull_one(json_message& message);
	void notice_pull_application(json_message& message);
	void notice_apply_pull_one(json_message& message);

	void item_exec(json_message& message);
	void item_pull(json_message& message);
	void item_modify(json_message& message);

	void user_exec(json_message& message);
	void user_pull(json_message& message);
	void user_modify(json_message& message);

	void message_send(std::uint64_t from_id,const std::string& content);

	void message_exec(json_message& message);
	void message_send(json_message& message);
	void message_pull(json_message& message);
	void message_pull_certain_user(json_message& message);

	void err_exec(int code, const std::string& content);
	void ws_new(std::string);
	void ws_read(std::string&);
	std::string ws_write(const std::string&);
private:
	bool started;
	bool is_ws_;
	std::unique_ptr<boost::asio::ip::tcp::socket> soc_;
	std::vector<char> read_buffer_;
	std::vector<char> write_buffer_;
	Server *server_;
	std::uint64_t user_id_;
	//std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> ws_;
	//boost::beast::multi_buffer buffer_;
};
