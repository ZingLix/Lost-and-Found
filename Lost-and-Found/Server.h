#pragma once
#include "boost/asio.hpp"
#include "DbConnector.h"

class User;

class Server
{
public:
	using err_code = boost::system::error_code;
	Server(unsigned short port);
	Server(boost::asio::ip::tcp::endpoint);
	void start();

	boost::asio::ip::tcp::acceptor& acceptor() { return acceptor_; }
	DbConnector& db() { return db_; }
	bool isOnline(std::uint64_t id);

	void handle_accept(const err_code& ec);

	void visitor_close(const std::shared_ptr<User>& use_ptr);
	void user_close(const std::shared_ptr<User>& use_ptr);
	void user_login(const std::shared_ptr<User>& use_ptr);

	std::shared_ptr<User> getUser(std::uint64_t id);

private:
	void updateAcceptor();

	boost::asio::io_context context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	std::unique_ptr<boost::asio::ip::tcp::socket> waiting_soc_;
	std::vector<std::shared_ptr<User>> vistor_list_;
	std::map<std::uint64_t, std::shared_ptr<User>> user_list_;
	DbConnector db_;
};