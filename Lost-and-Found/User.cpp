#include "User.h"
#include "boost/asio.hpp"
#include "msg.h"
#include "LogInfo.h"

using namespace boost::asio;

User::User(socket_ptr&& soc_ptr,Server* s)
	:started(false),soc_(std::move(soc_ptr)),
	read_buffer_(1024),write_buffer_(1024),server_(s),
	user_id_(0)
{}

void User::start() {
	started = true;
	do_read();
}

void User::stop() {
	if (started == true) {
		started = false;
		soc_->shutdown(socket_base::shutdown_both);

		server_->user_close(shared_from_this());
	}
}

User::socket_ptr& User::socket() {
	return soc_;
}

void User::on_read(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		stop();
		return;
	}
	const std::string msg(&*read_buffer_.begin(), bytes);
	try {
		json_message message(msg);
		msg_exec(message);
	}catch ( std::invalid_argument& e ) {
		err_exec(1, e.what());
		stop();
	}
}

void User::on_write(const boost::system::error_code& err, size_t bytes) {
	do_read();
}


void User::do_read() {
	if (started == false) return;
	using namespace std::placeholders;
	soc_->async_read_some(buffer(read_buffer_), 
		std::bind(&User::on_read, this, _1, _2));
}

void User::do_write(const std::string& str) {
	if (started == false) return;
	using namespace std::placeholders;
	
	std::copy(str.begin(), str.end(), &*write_buffer_.begin());
	soc_->async_write_some(buffer(write_buffer_, str.size()), 
		std::bind(&User::on_write,this,_1,_2));
}

User::~User() {
	stop();
	soc_->close();
	LOG_DEBUG <<user_id_<< " destroyed";
}

void User::msg_exec(json_message& msg) {
	switch ( msg.getInt("type") ) {
	case 1:
		user_login(msg);
		break;
	case 2:
		user_register(msg);
		break;
	case 11:
		notice_exec(msg);
		break;
	default:
		stop();
	}
}

void User::user_login(json_message& message) {
	std::uint64_t id = server_->db().checkPassword(message.getString("username"), message.getString("password"));
	if (id != 0)
		login_success(id);
	else
		login_failure("Incorrect username or password.");
}

void User::login_success(std::uint64_t id) {
	json_message msg;
	msg.add("type", 1);
	msg.add("code", 1);
	msg.add("user_id", id);
	user_id_ = id;
	do_write(msg.getString());
}

void User::login_failure(const std::string& reason) {
	json_message msg;
	msg.add("type", 1);
	msg.add("code", 2);
	msg.add("content", reason);
	do_write(msg.getString());
	stop();
}

void User::err_exec(int code, const std::string& content) {
	json_message msg;
	msg.add("type", 9);
	msg.add("code", code);
	msg.add("content", content);
	do_write(msg.getString());
}

void User::user_register(json_message& message) {
	auto result = server_->db().registerNewUser(message.getString("username"), message.getString("password"));
	json_message msg;
	msg.add("type", 2);
	if(result.first==true) {
		msg.add("code", 1);
	}else {
		msg.add("code", 2);
		msg.add("content", result.second);
	}
	do_write(msg.getString());
	stop();
}

void User::notice_exec(json_message& message) {
	switch (message.getInt("code")) {
	case 1:
		notice_post(message);
		break;
	case 2:
		notice_pull(message);
		break;
	default:
		break;
	}
}


void User::notice_post(json_message& message) {
	std::uint64_t finder_id = user_id_;
	std::uint64_t item_id = server_->db().addItem(message.getString("item_name"), message.getString("item_info"), message.getString("lost_location"));
	std::uint64_t notice_id = server_->db().addNotice(finder_id, item_id);
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 11);
	msg.add("notice_id", notice_id);
	do_write(msg.getString());
}

void User::notice_pull(json_message& message) {
	auto result = server_->db().queryNotice();
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 12);
	rapidjson::Value arr(rapidjson::kArrayType);
	for(auto & res:result) {
		rapidjson::Value a(rapidjson::kArrayType);
		a.PushBack(rapidjson::Value (std::get<0>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value (std::get<1>(res).c_str(), msg.getAllocator()), msg.getAllocator());
		a.PushBack(rapidjson::Value (std::get<2>(res)), msg.getAllocator());
		arr.PushBack(a, msg.getAllocator());
	}
	msg.add("notice_info", arr);
	do_write(msg.getString());
}
