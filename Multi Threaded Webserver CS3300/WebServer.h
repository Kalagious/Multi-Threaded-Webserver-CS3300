#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <string>
#include <regex>
#include "User.h"
#include "Message.h"
#include <vector>

#pragma comment (lib, "Ws2_32.lib")

class WebServer {
	private:
		
	
	
		int port = 8080;
		bool isRunning = false;
		int userCount = 0;
		int messageCount = 0;

		const int maxUsers = 1000;
		const int maxMessages = 10000;
		const int maxSessions = 100;

		std::vector<User*> users;
		std::vector<Message*> messages;

		SOCKET serverSocket;

		void waitForConnections();
		void handleClient(SOCKET clientSocket);

		std::string boilerplate(std::string content , int code = 200);
		std::string generate301Redirect(std::string location, std::string extraHeaders = "");

		int identifyRequest(std::string request);
		std::string handleGET(std::string request);
		std::string handlePOST(std::string request);
		User* sessionToUser(std::string sessionID);
		void sendMessage(User* sender, std::string content);
		int createUser(std::string username, std::string password);
		std::string newSession(User* user);
		User* authenticate(std::string username, std::string password);
		std::string getCookie(std::string request, std::string cookieName);

		std::string sanitizeInput(std::string input);
		std::string urlDecode(std::string input);


		// Pages
		std::string loginPage();
		std::string logoutPage(User* user);
		std::string registerPage();
		std::string messagesPage(User* user);


		std::string loginPost(std::string data);
		std::string registerPost(std::string data);
		std::string messagesPost(std::string data, User* user);



	public:
		WebServer(int port);
		void start();
		void stop();
};