#pragma once
#include "User.h"
#include <string>



class Message {
	public:
		User* sender;
		uint64_t timestamp;
		std::string content;
		Message(User* sndr, std::string cntnt) : sender(sndr), content(cntnt) {
			timestamp = static_cast<uint64_t>(time(nullptr));
		}
};