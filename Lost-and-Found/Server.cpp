#include "Server.h"
#include "User.h"
#include "LogInfo.h"

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
	db_.connect("zinglix", "password");
	updateAcceptor();
	LOG_INFO << "Server running on "<< acceptor_.local_endpoint().port();
	context_.run();
}

void Server::updateAcceptor() {
	acceptor().async_accept(*waiting_soc_, std::bind(&Server::handle_accept, this, std::placeholders::_1));
}

void Server::handle_accept(const err_code& ec) {
	LOG_INFO << "New connect from " << waiting_soc_->remote_endpoint().address().to_string() << ":" << waiting_soc_->remote_endpoint().port();
	vistor_list_.push_back( std::make_shared<User>(std::move(waiting_soc_),this));
	waiting_soc_ = std::make_unique<ip::tcp::socket>(context_);
	vistor_list_.back()->start();
	updateAcceptor();
}

void Server::visitor_close(const std::shared_ptr<User>& user) {
	LOG_INFO << user->socket()->remote_endpoint().address().to_string() << ":" << user->socket()->remote_endpoint().port() << " disconnected.";
	vistor_list_.erase(std::find(vistor_list_.begin(),vistor_list_.end(),user));
}

void Server::user_close(const std::shared_ptr<User>& user) {
	user_list_.erase(user_list_.find(user->id()));
}

void Server::user_login(const std::shared_ptr<User>& user) {
	vistor_list_.erase(std::find(vistor_list_.begin(), vistor_list_.end(), user));
	auto tmp = user_list_.find(user->id());
	if(tmp!=user_list_.end()) {
		tmp->second->err_exec(2, "Another client go online.");
		tmp->second->stop();
	}
	user_list_.emplace(user->id(), user);
}

bool Server::isOnline(std::uint64_t id) {
	return user_list_.find(id) != user_list_.end();
}

std::shared_ptr<User> Server::getUser(std::uint64_t id) {
	return user_list_[id];
}
