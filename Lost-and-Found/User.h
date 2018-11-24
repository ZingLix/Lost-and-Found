#pragma once
#include "boost/asio.hpp"
#include <memory>
#include <vector>
#include "Server.h"

class User:public std::enable_shared_from_this<User>
{
public:
	using socket_ptr = std::unique_ptr<boost::asio::ip::tcp::socket>;

	User(socket_ptr&& soc_ptr,Server* s);
	virtual ~User();
	void start();
	void stop();

	void on_write(const boost::system::error_code & err, size_t bytes);
	void on_read(const boost::system::error_code & err, size_t bytes);
	void do_read();
	void do_write(const std::string& str);

private:
	bool started;
	std::unique_ptr<boost::asio::ip::tcp::socket> soc_;
	std::vector<char> read_buffer_;
	std::vector<char> write_buffer_;
	Server *server_;
};
