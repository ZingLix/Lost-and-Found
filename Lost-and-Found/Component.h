#pragma once
#include <string>

struct item
{
	item(std::uint64_t id,std::string name,std::string info,std::string location)
		:item_id(id),item_name(name),item_info(info),lost_location(location)
	{}

	std::uint64_t item_id;
	std::string item_name;
	std::string item_info;
	std::string lost_location;
};


struct userinfo
{
	userinfo(std::uint64_t id, std::string email_, std::string phone_, std::string description_)
		:user_id(id), email(email_),phone(phone_),description(description_)
	{}

	std::uint64_t user_id;
	std::string email;
	std::string phone;
	std::string description;
};