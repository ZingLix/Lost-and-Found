#include "Server.h"
#include "User.h"
#include <iostream>

using namespace boost::asio;

Server::Server(unsigned short port)
	:acceptor_(context_,ip::tcp::endpoint(ip::tcp::v4(),port)),
	waiting_soc_(std::make_unique<ip::tcp::socket>(context_))
{
}


Server::Server(ip::tcp::endpoint ep)
	:acceptor_(context_,ep),waiting_soc_(std::make_unique<ip::tcp::socket>(context_))
{
}


void Server::start() {
	updateAcceptor();
	context_.run();
}

void Server::updateAcceptor() {
	acceptor().async_accept(*waiting_soc_, std::bind(&Server::handle_accept, this, std::placeholders::_1));
}

void Server::handle_accept(const err_code& ec) {
	user_list_.push_back( std::make_shared<User>(std::move(waiting_soc_),this));
	waiting_soc_ = std::make_unique<ip::tcp::socket>(context_);
	user_list_.back()->start();
	updateAcceptor();
	std::cout << "user login." << std::endl;
}

void Server::user_close(const std::shared_ptr<User>& user_ptr) {
	std::cout << "user logout." << std::endl;
	user_list_.erase(std::find(user_list_.begin(),user_list_.end(),user_ptr));
}
