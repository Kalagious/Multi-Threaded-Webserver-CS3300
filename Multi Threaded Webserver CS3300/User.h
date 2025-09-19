#pragma once
#include <string>



class User {

public:
	std::string username;
	std::string hash;
	std::string sessionID;
	User(std::string uname, std::string pwhash) : username(uname), hash(pwhash) {}
};

