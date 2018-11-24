#include "User.h"
#include "boost/asio.hpp"
#include <iostream>

using namespace boost::asio;

User::User(socket_ptr&& soc_ptr,Server* s)
	:started(false),soc_(std::move(soc_ptr)),
	read_buffer_(1024),write_buffer_(1024),server_(s) {

}

void User::start() {
	started = true;
	do_read();
}

void User::stop() {
	started = false;
	(*soc_).shutdown(socket_base::shutdown_both);
	(*soc_).close();
	server_->user_close(shared_from_this());
}

void User::on_read(const boost::system::error_code & err, size_t bytes) {
	if (err == error::eof) stop();

	const std::string msg(&*read_buffer_.begin(), bytes);
	do_write(msg + "\n");
	std::cout << "echo message: " << msg ;
}

void User::on_write(const boost::system::error_code& err, size_t bytes) {
	do_read();
}


void User::do_read() {
	using namespace std::placeholders;
	(*soc_).async_read_some(buffer(read_buffer_), 
		std::bind(&User::on_read, this, _1, _2));
}

void User::do_write(const std::string& str) {
	if (started == false) return;
	using namespace std::placeholders;
	
	std::copy(str.begin(), str.end(), &*write_buffer_.begin());
	(*soc_).async_write_some(buffer(write_buffer_, str.size()), 
		std::bind(&User::on_write,this,_1,_2));
}

User::~User() {
	std::cout << "user destroyed" << std::endl;
}
