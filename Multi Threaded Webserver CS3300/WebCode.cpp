#include "WebServer.h"
#include "User.h"
#include "sha512.hh"
#include "UUIDGenerate.hh"

void WebServer::handleClient(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived;
    std::string response;

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
		std::string request(buffer, bytesReceived);

		printf(" [*] Received request:\n%s\n", request.c_str());
		int requestType = identifyRequest(request);

		if (requestType == 1) 
			response = handleGET(request);
		else if (requestType == 2)
			response = handlePOST(request);
        
        if (response.length() > 0)
            send(clientSocket, response.c_str(), std::strlen(response.c_str()), 0);

    }

    closesocket(clientSocket);
}

std::string WebServer::handleGET(std::string request) {

	std::string sessionCookie = getCookie(request, "sessionID");
	User* currentUser = sessionToUser(sessionCookie);

	std::string content;
	std::regex endpointRegex("^GET \/([a-zA-Z\/]*)");
	std::smatch match;

    if (std::regex_search(request, match, endpointRegex)) {
        std::string endpoint = match[1];
        if (endpoint.size())
            printf("    Requested endpoint: %s\n", endpoint.c_str());

        if (!currentUser && (endpoint == "" || endpoint == "login"))
            content = boilerplate(loginPage());
        else if (!currentUser && endpoint == "register")
            content = boilerplate(registerPage());
        else if (endpoint == "logout")
            content = boilerplate(logoutPage(currentUser));
        else if (currentUser)
            content = boilerplate(messagesPage(currentUser));
        else
            content = boilerplate("<html><body><h1>404 Not Found</h1></body></html>", 404);
    } else {
		content = boilerplate("<html><body><h1>400 Bad Request</h1></body></html>", 400);
    }

   
        
    
    return content;
}

std::string WebServer::handlePOST(std::string request) {

    std::string sessionCookie = getCookie(request, "sessionID");
    User* currentUser = sessionToUser(sessionCookie);


    std::regex endpointRegex("^POST /(\\S*) ");
	std::smatch endpointMatch;
	std::regex dataRegex("\r\n\r\n(.*)$");
	std::smatch dataMatch;
	std::string response;

    if (std::regex_search(request, endpointMatch, endpointRegex) && std::regex_search(request, dataMatch, dataRegex)) {
        std::string endpoint = endpointMatch[1];
        std::string data = dataMatch[1];


        if (!currentUser && endpoint == "login") 
            response = loginPost(data);
        
        else if (!currentUser && endpoint == "register") 
            response = registerPost(data);
        
        else if (currentUser && endpoint == "send") 
			response = messagesPost(data, currentUser);
		
        else
            response = generate301Redirect("/");
        
    } 
    else 
        response = boilerplate("<html><body><h1>400 Bad Request</h1></body></html>", 400);
	


    return response;
}



int WebServer::createUser(std::string username, std::string password) {

	// Check if the max user count has been reached or if the username already exists
    if (userCount >= maxUsers) 
        return -2;
	

    for (int i = 0; i < userCount; i++) 
        if (users[i]->username == username) 
            return -1;
        
   
	// Calculate the SHA-512 hash of the password and store the new user
    std::string hash = sw::sha512::calculate(password);
    User* newUser = new User(username, hash);
	users.push_back(newUser);
    printf("    Created user: %s\n", username.c_str());
    return 1;
}


std::string WebServer::newSession(User* user) {
	// Generate a new UUID v4 and assign it to the user
	std::string sessionID = uuid::generate_uuid_v4();
    user->sessionID = sessionID;
    printf("    Created session for user %s: %s\n", user->username.c_str(), sessionID.c_str());
    return sessionID;
}


User* WebServer::sessionToUser(std::string sessionID) {
	// Make sure a null session ID returns doesnt bypass authentication
    if (sessionID.length() == 0) 
		return nullptr;

    // Find if any user has the given session ID
    for (int i = 0; i < users.size(); i++) 
        if (users.at(i)->sessionID == sessionID) 
            return users[i];
       
    return nullptr;
}


User* WebServer::authenticate(std::string username, std::string password) {
	// Calculate the SHA-512 hash of the provided password and find if any user names and passwords match
    std::string hash = sw::sha512::calculate(password);


    for (int i = 0; i < users.size(); i++) {
        if (users.at(i)->username == username && users.at(i)->hash == hash) {
            printf("    Authenticated user: %s\n", username.c_str());
            return users[i];
        }
    }
    return nullptr;
}

void WebServer::sendMessage(User* sender, std::string content) {
    if (messageCount >= maxMessages) 
        return;
    Message* newMessage = new Message(sender, content);
    messages.push_back(newMessage);
    messageCount++;
    printf("    New message from %s: %s\n", sender->username.c_str(), content.c_str());
}

