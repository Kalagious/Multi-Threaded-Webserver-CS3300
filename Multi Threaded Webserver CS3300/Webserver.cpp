#include "WebServer.h"


WebServer::WebServer(int portIn) {
	port = portIn;
	
	printf(" [*] WebServer created on port %d\n", port);
}


void WebServer::start() {
	printf(" [*] Starting server on port %d...\n", port);
    
    

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	listen(serverSocket, SOMAXCONN);



	isRunning = true;
	printf(" [*] Server started. Waiting for connections on port %d...\n", port);

	createUser("admin", "password");
	sendMessage(users[0], "Welcome to the C++ Webserver!");

	waitForConnections();

	stop();
}


void WebServer::stop() {

	printf(" [!] Stopping server...\n");

	closesocket(serverSocket);
	WSACleanup();

	isRunning = false;
}


void WebServer::waitForConnections() {
	while (isRunning) {

		sockaddr_in clientAddress;
		int clientSize = sizeof(clientAddress);

		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientSize);

		if (clientSocket != INVALID_SOCKET) {
			std::thread clientThread(&WebServer::handleClient, this, clientSocket);
			clientThread.detach();
		}
	}
}

std::string WebServer::boilerplate(std::string content, int code) {
    std::string response = "HTTP/1.1 ";
	response += std::to_string(code) + " ";
    if (code == 200) 
        response += " OK\r\n";
	else if (code == 400)
		response += " Bad Request\r\n";
	else if (code == 401)
		response += " Unauthorized\r\n";
	else if (code == 404)
		response += " Not Found\r\n";
	else
		response += " Internal Server Error\r\n";


    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += content;

    return response;
}

std::string WebServer::generate301Redirect(std::string location, std::string extraHeaders) {
	std::string response = "HTTP/1.1 301 Moved Permanently\r\n";
	response += extraHeaders;
	response += "Location: " + location + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	return response;
}



int WebServer::identifyRequest(std::string request) {
    std::regex getRegex("^GET ");
    std::regex postRegex("^POST ");
    if (std::regex_search(request, getRegex))
        return 1;
    else if (std::regex_search(request, postRegex))
        return 2;
    else
        return -1;
}

std::string WebServer::getCookie(std::string request, std::string cookieName) {

	// Use regex to find a cookie with the given name in the request headers
	std::regex cookieRegex("Cookie: (.*)");
	std::smatch match;
	if (std::regex_search(request, match, cookieRegex)) {
		std::string cookies = match[1];
		std::regex specificCookieRegex(cookieName + "=([^;]*)");
		if (std::regex_search(cookies, match, specificCookieRegex)) {
			return match[1];
		}
	}
	return "";
}