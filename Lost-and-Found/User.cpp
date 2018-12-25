#include "User.h"
#include "boost/asio.hpp"
#include "msg.h"
#include "LogInfo.h"
#include "Component.h"
#include "openssl/sha.h"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <arpa/inet.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include  <cstdint> 
using namespace boost::asio;
using namespace boost::beast;
User::User(socket_ptr&& soc_ptr, Server* s)
	:started(false), is_ws_(false), soc_(std::move(soc_ptr)),
	read_buffer_(1024), write_buffer_(1024), server_(s),
	user_id_(0)
{}

void User::start() {
	started = true;
	do_read();
}

void User::stop() {
	if (started == true) {
		started = false;
		LOG_INFO << socket()->remote_endpoint().address().to_string() << ":" << socket()->remote_endpoint().port() << " disconnected.";
		//LOG_INFO << user_id_ << " disconnected.";

		soc_->shutdown(socket_base::shutdown_both);


		if (user_id_ == 0)
			server_->visitor_close(shared_from_this());
		else
			server_->user_close(shared_from_this());
	}
}

User::socket_ptr& User::socket() {
	return soc_;
}

std::uint64_t User::id() {
	return user_id_;
}

void User::on_read(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		if (err == boost::asio::error::eof) {
			LOG_INFO << user_id_ << " end of file";
		}
		stop();
		return;
	}
	std::string msg(&*read_buffer_.begin(), bytes);
	if (msg.find("GET") != std::string::npos && is_ws_ == false) {
		//	using namespace std::placeholders;
		ws_new(msg);
		//	ws_ = std::make_unique<websocket::stream<ip::tcp::socket>>(std::move(*soc_));
		//	soc_.reset();
		return;
	}
	std::vector<std::string> msg_set;
	if (is_ws_) {
		msg_set = ws_read(msg);
	}
	else {
		msg_set.push_back(msg);
	}
	for (auto&messages : msg_set) {
		try {
			json_message message(messages);
			msg_exec(message);
		}
		catch (std::invalid_argument& e) {
			LOG_ERROR << user_id_ << ": " << e.what();
			err_exec(1, e.what());
			stop();
		}
	}

}

const std::string base64_padding[] = { "", "==","=" };
std::string Base64Encode(const std::string& s)
{
	namespace bai = boost::archive::iterators;

	std::stringstream os;

	// convert binary values to base64 characters
	typedef bai::base64_from_binary
		// retrieve 6 bit integers from a sequence of 8 bit bytes
		<bai::transform_width<const char *, 6, 8> > base64_enc; // compose all the above operations in to a new iterator

	std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
		std::ostream_iterator<char>(os));

	os << base64_padding[s.size() % 3];
	return os.str();
}

void User::ws_new(std::string msg) {
	auto it = msg.find("Sec-WebSocket-Key: ");
	std::string key(msg.substr(it + 19, 43 - 19));
	key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	unsigned char hash[20];
	SHA_CTX s;
	SHA1_Init(&s);
	SHA1_Update(&s, key.c_str(), key.length());
	SHA1_Final(hash, &s);
	key = std::string(reinterpret_cast<char*>(hash), 20);

	key = Base64Encode(key);

	std::string response;
	response += "HTTP/1.1 101 Switching Protocols\r\n";
	response += "Connection: upgrade\r\n";
	response += "Sec-WebSocket-Accept: ";
	response += key + "\r\n";
	response += "Upgrade: websocket\r\n\r\n";
	do_write(response);
	is_ws_ = true;
}


void User::on_write(const boost::system::error_code& err, size_t bytes) {
	do_read();
}


void User::do_read() {
	if (started == false) return;
	using namespace std::placeholders;
	//if(is_ws_) {
	//	ws_->async_read_some(buffer(read_buffer_),
	//		std::bind(&User::on_read, this, _1, _2));
	//}
	//else {
	soc_->async_read_some(buffer(read_buffer_),
		std::bind(&User::on_read, this, _1, _2));
	//}
}

void User::do_write(const std::string& str) {
	if (started == false) return;
	std::string s(ws_write(str));
	using namespace std::placeholders;
	std::copy(s.begin(), s.end(), &*write_buffer_.begin());
	//	if(is_ws_) {
		//	ws_->async_write(buffer_.data(), std::bind(&User::on_write, this, _1, _2));
		//	ws_->async_write_some(buffer(write_buffer_, str.size()),
		//		std::bind(&User::on_write, this, _1, _2));
	//	}
	//	else {
	soc_->async_write_some(buffer(write_buffer_, s.size()),
		std::bind(&User::on_write, this, _1, _2));
	//	}
}

std::string User::ws_write(const std::string& str) {
	if (!is_ws_) return str;
	std::string s;
	s += 0x81;
	if (str.length() <= 125) s += str.length();
	else if (str.length() <= 65535) {
		s += 126;
		std::uint16_t l = static_cast<std::uint16_t>(str.length());
		for (int i = 8; i >= 0; i -= 8) {
			auto tmp = l >> i;
			tmp &= 0x0ff;
			s += tmp;
		}
	}
	else {
		s += 127;
		std::uint16_t l = static_cast<std::uint16_t>(str.length());
		for (int i = 64 - 8; i >= 0; i -= 8) {
			auto tmp = l >> i;
			tmp &= 0x0ff;
			s += tmp;
		}
	}
	s += str;
	return s;
}


User::~User() {
	stop();
	soc_->close();
	LOG_DEBUG << user_id_ << " destroyed";
}

void User::msg_exec(json_message& msg) {
	switch (msg.getInt("type")) {
	case 1:
		user_login(msg);
		break;
	case 2:
		user_register(msg);
		break;
	case 3:
		item_exec(msg);
		break;
	case 4:
		user_exec(msg);
		break;
	case 5:
		message_exec(msg);
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
	user_id_ = id;
	server_->user_login(shared_from_this());
	json_message msg;
	msg.add("type", 1);
	msg.add("code", 1);
	msg.add("user_id", id);
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
	if (result.first == true) {
		msg.add("code", 1);
	}
	else {
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
	case 3:
		notice_claim(message);
		break;
	case 4:
		notice_apply_pull(message);
		break;
	case 5:
		application_status_update(message);
		break;
	case 6:
		notice_withdraw(message);
		break;
	case 7:
		application_withdraw(message);
		break;
	case 8:
		notice_search(message);
		break;
	case 9:
		notice_query(message);
		break;
	case 10:
		notice_pull_one(message);
		break;
	case 21:
		notice_pull_application(message);
		break;
	case 22:
		notice_apply_pull_one(message);
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
	for (auto & res : result) {
		rapidjson::Value a(rapidjson::kArrayType);
		a.PushBack(rapidjson::Value(std::get<0>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<1>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<2>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<3>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<4>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<5>(res).c_str(), msg.getAllocator()), msg.getAllocator());
		arr.PushBack(a, msg.getAllocator());
	}
	msg.add("notice_info", arr);
	do_write(msg.getString());
}

void User::notice_claim(json_message& message) {
	std::uint64_t app_seq = server_->db().addApplication(user_id_, message.getUInt64("notice_id"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 13);
	msg.add("application_seq", app_seq);
	do_write(msg.getString());
}

void User::notice_apply_pull(json_message& message) {
	auto result = server_->db().queryApplication(user_id_);
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 14);
	rapidjson::Value arr(rapidjson::kArrayType);
	for (auto & res : result) {
		rapidjson::Value a(rapidjson::kArrayType);
		a.PushBack(rapidjson::Value(std::get<0>(res)), msg.getAllocator());//application_seq
		a.PushBack(rapidjson::Value(std::get<1>(res)), msg.getAllocator());//applicant_id
		a.PushBack(rapidjson::Value(std::get<2>(res)), msg.getAllocator());//notice_id
		a.PushBack(rapidjson::Value(std::get<3>(res)), msg.getAllocator());//status
		a.PushBack(rapidjson::Value(std::get<4>(res).c_str(), msg.getAllocator()), msg.getAllocator());//time
		arr.PushBack(a, msg.getAllocator());
	}
	msg.add("application_info", arr);
	do_write(msg.getString());
}

void User::application_status_update(json_message& message) {
	server_->db().execApplication(message.getUInt64("application_seq"), message.getInt("status"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 15);
	do_write(msg.getString());
}

void User::notice_withdraw(json_message& message) {
	server_->db().withdrawNotice(message.getUInt64("notice_id"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 16);
	msg.add("notice_id", message.getUInt64("notice_id"));
	do_write(msg.getString());
}

void User::application_withdraw(json_message& message) {
	server_->db().withdrawApplication(message.getUInt64("application_seq"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 17);
	msg.add("application_seq", message.getUInt64("application_seq"));
	do_write(msg.getString());
}

void User::item_exec(json_message& message) {
	switch (message.getInt("code")) {
	case 1:
		item_pull(message);
		break;
	case 2:
		item_modify(message);
		break;
	default:
		break;
	}
}

void User::item_pull(json_message& message) {
	auto i = server_->db().queryItem(message.getUInt64("item_id"));
	json_message msg;
	msg.add("type", 3);
	msg.add("code", 11);
	msg.add("item_id", i.item_id);
	msg.add("item_name", i.item_name);
	msg.add("item_info", i.item_info);
	msg.add("lost_location", i.lost_location);
	do_write(msg.getString());
}

void User::item_modify(json_message& message) {
	item i(message.getUInt64("item_id"), message.getString("item_name"),
		message.getString("item_info"), message.getString("lost_location"));
	server_->db().modifyItem(i);
	json_message msg;
	msg.add("type", 3);
	msg.add("code", 12);
	do_write(msg.getString());
}

void User::user_exec(json_message& message) {
	switch (message.getInt("code")) {
	case 1:
		user_pull(message);
		break;
	case 2:
		user_modify(message);
		break;
	default:
		break;
	}
}

void User::user_pull(json_message& message) {
	auto i = server_->db().queryUser(message.getUInt64("user_id"));
	json_message msg;
	msg.add("type", 4);
	msg.add("code", 11);
	msg.add("user_id", i.user_id);
	msg.add("username", i.username);
	msg.add("email", i.email);
	msg.add("phone", i.phone);
	msg.add("description", i.description);
	do_write(msg.getString());
}

void User::user_modify(json_message& message) {
	userinfo i(message.getUInt64("user_id"), message.getString("username"), message.getString("email"),
		message.getString("phone"), message.getString("description"));
	server_->db().modifyUser(i);
	json_message msg;
	msg.add("type", 4);
	msg.add("code", 12);
	do_write(msg.getString());
}

void User::notice_search(json_message& message) {
	auto result = server_->db().queryNotice(message.getString("keyword"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 18);
	rapidjson::Value arr(rapidjson::kArrayType);
	for (auto & res : result) {
		rapidjson::Value a(rapidjson::kArrayType);
		a.PushBack(rapidjson::Value(std::get<0>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<1>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<2>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<3>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<4>(res)), msg.getAllocator());
		a.PushBack(rapidjson::Value(std::get<5>(res).c_str(), msg.getAllocator()), msg.getAllocator());
		arr.PushBack(a, msg.getAllocator());
	}
	msg.add("notice_info", arr);
	do_write(msg.getString());
}

void User::message_send(std::uint64_t from_id, std::uint64_t msg_seq, const std::string& content) {
	json_message msg;
	msg.add("type", 5);
	msg.add("code", 0);
	msg.add("msg_seq", msg_seq);
	msg.add("sender_id", from_id);
	msg.add("content", content);
	do_write(msg.getString());
}

void User::message_exec(json_message& message) {
	switch (message.getInt("code")) {
	case 1:
		message_send(message);
		break;
	case 2:
		message_pull(message);
		break;
	case 3:
		message_pull_certain_user(message);
		break;
	default:
		break;
	}
}

void User::message_send(json_message& message) {
	std::uint64_t id = message.getUInt64("recver_id");
	auto seq = server_->db().addMessageRecord(user_id_, id, message.getString("content"));
	if (server_->isOnline(id)) {
		server_->getUser(id)->message_send(user_id_, seq, message.getString("content"));
	}
	json_message msg;
	msg.add("type", 5);
	msg.add("code", 11);
	msg.add("recver_id", message.getUInt64("recver_id"));
	msg.add("msg_seq", seq);
	msg.add("content", message.getString("content"));
	do_write(msg.getString());
}

void User::message_pull(json_message& message) {
	auto messages = server_->db().pullMessageRecord(user_id_);
	json_message msg;
	msg.add("type", 5);
	msg.add("code", 12);
	rapidjson::Value arr(rapidjson::kArrayType);
	for (auto& m : messages) {
		rapidjson::Value a(rapidjson::kArrayType);
		a.PushBack(rapidjson::Value(m.msg_seq_id), msg.getAllocator());
		if (m.sender_id == user_id_) {
			a.PushBack(rapidjson::Value(0), msg.getAllocator());
			a.PushBack(rapidjson::Value(m.recver_id), msg.getAllocator());
		}
		else {
			a.PushBack(rapidjson::Value(1), msg.getAllocator());
			a.PushBack(rapidjson::Value(m.sender_id), msg.getAllocator());
		}
		a.PushBack(rapidjson::Value(m.content, msg.getAllocator()), msg.getAllocator());
		arr.PushBack(a, msg.getAllocator());
	}
	msg.add("messages", arr);
	do_write(msg.getString());
}

void User::message_pull_certain_user(json_message& message) {
	auto messages = server_->db().pullMessageRecord(user_id_);
	json_message msg;
	msg.add("type", 5);
	msg.add("code", 13);
	msg.add("target_user", message.getUInt64("user_id"));
	rapidjson::Value arr(rapidjson::kArrayType);
	for (auto& m : messages) {
		rapidjson::Value a(rapidjson::kArrayType);
		a.PushBack(rapidjson::Value(m.msg_seq_id), msg.getAllocator());
		if (m.sender_id == user_id_) {
			a.PushBack(rapidjson::Value(0), msg.getAllocator());
		}
		else {
			a.PushBack(rapidjson::Value(1), msg.getAllocator());
		}
		a.PushBack(rapidjson::Value(m.content, msg.getAllocator()), msg.getAllocator());
		arr.PushBack(a, msg.getAllocator());
	}
	msg.add("messages", arr);
	do_write(msg.getString());
}

std::vector<std::string> User::ws_read(std::string& str) {
	std::vector<std::string> msg_set;
	int pos = 0;
	while (pos != str.length()) {
		uint fin = (unsigned char)str[pos] >> 7;
		uint opcode_ = str[pos++] & 0x0f;
		uint mask = (unsigned char)str[pos] >> 7;

		uint payload_length_ = str[pos] & 0x7f;
		pos++;
		if (payload_length_ == 126) {
			uint16_t length = 0;
			memcpy(&length, str.c_str() + pos, 2);
			pos += 2;
			payload_length_ = ::ntohl(length);
		}
		else if (payload_length_ == 127) {
			uint64_t length = 0;
			memcpy(&length, str.c_str() + pos, 8);
			pos += 4;
			payload_length_ = ::ntohl(length);
		}

		std::array<char, 4> mask_key;
		for (int i = 0; i < 4; i++)
			mask_key[i] = str[pos + i];
		pos += 4;

		std::array<char, 1024> data;
		if (mask != 1) {
			std::copy(str.begin() + pos, str.end(), data.begin());
		}
		else {
			for (size_t i = 0; i < payload_length_; i++) {
				int j = i % 4;
				data[i] = str[pos + i] ^ mask_key[j];
			}
		}
		pos += payload_length_;
		msg_set.push_back(std::string(data.begin(), data.begin() + payload_length_));
	}
	return msg_set;
}

void User::notice_query(json_message& message) {
	auto res = server_->db().queryNotice(message.getUInt64("notice_id"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 19);
	msg.add("notice_id", message.getUInt64("notice_id"));
	rapidjson::Value a(rapidjson::kArrayType);
	a.PushBack(rapidjson::Value(std::get<0>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<1>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<2>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<3>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<4>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<5>(res).c_str(), msg.getAllocator()), msg.getAllocator());
	msg.add("notice_info", a);
	do_write(msg.getString());
}

void User::notice_pull_one(json_message& message)
{
	auto res = server_->db().queryNotice_one(user_id_);
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 20);
	rapidjson::Value a(rapidjson::kArrayType);
	for (auto n : res)
	{
		a.PushBack(rapidjson::Value(n), msg.getAllocator());
	}
	msg.add("notice_list", a);
	do_write(msg.getString());
}

void User::notice_pull_application(json_message& message) {
	auto res = server_->db().queryNotice_whoapply(message.getUInt64("notice_id"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 31);
	msg.add("notice_id", message.getUInt64("notice_id"));
	rapidjson::Value a(rapidjson::kArrayType);
	for (auto n : res) {
		a.PushBack(rapidjson::Value(n), msg.getAllocator());
	}
	msg.add("application_list", a);
	do_write(msg.getString());
}

void User::notice_apply_pull_one(json_message& message) {
	auto res = server_->db().queryApplication_one(message.getUInt64("application_seq"));
	json_message msg;
	msg.add("type", 11);
	msg.add("code", 32);
	rapidjson::Value a(rapidjson::kArrayType);
	a.PushBack(rapidjson::Value(std::get<0>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<1>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<2>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<3>(res)), msg.getAllocator());
	a.PushBack(rapidjson::Value(std::get<4>(res).c_str(), msg.getAllocator()), msg.getAllocator());
	msg.add("application_info", a);
	do_write(msg.getString());
}
