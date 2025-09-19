#include "WebServer.h"
#include <fstream>

std::string WebServer::loginPage()
{
	std::ifstream file("Pages/login.html");
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
    return content;
}

std::string WebServer::logoutPage(User* user)
{
    if (user)
        user->sessionID = "";

    return generate301Redirect("/");
}

	


std::string WebServer::registerPage()
{

    std::ifstream file("Pages/register.html");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content;
}


std::string WebServer::messagesPage(User* user)
{
    std::ifstream file("Pages/messages.html");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	content = std::regex_replace(content, std::regex("\\{USERNAME\\}"), user->username);
    
    
	std::ifstream messageFile("Pages/message.html");
    std::string messageTemplate((std::istreambuf_iterator<char>(messageFile)), std::istreambuf_iterator<char>());

	std::string messageHTML = "";

    for (int i = 0; i < messages.size(); i++)
    {
		std::string singleMessage = messageTemplate;
        singleMessage = std::regex_replace(singleMessage, std::regex("\\{SENDER\\}"), messages[i]->sender->username);
        singleMessage = std::regex_replace(singleMessage, std::regex("\\{CONTENT\\}"), messages[i]->content);
		messageHTML += singleMessage;
    }
	content = std::regex_replace(content, std::regex("\\{MESSAGES\\}"), messageHTML);
    
    return content;
}

std::string WebServer::loginPost(std::string data)
{
	// Use regex to extract username and password from data
	std::regex loginRegex("username=([^&]*)&password=([^&]*)");
	std::smatch match;
    if (std::regex_search(data, match, loginRegex)) {
        std::string username = match[1];
        std::string password = match[2];

		// See if the credentials are valid
        User* user = authenticate(username, password);
        if (user) {
            std::string sessionID = newSession(user);
            return generate301Redirect("/", "Set-Cookie: sessionID=" + sessionID + "; HttpOnly\r\n");
        }
        else 
			return generate301Redirect("/?error=invalidcreds");

    } else {
        return generate301Redirect("/?error=");
	}
    
}

std::string WebServer::registerPost(std::string data)
{
    std::regex registerRegex("username=([^&]*)&password=([^&]*)");
    std::smatch match;
    if (std::regex_search(data, match, registerRegex)) {
        std::string username = match[1];
        std::string password = match[2];
        int result = createUser(username, password);
        if (result == 1)
            return generate301Redirect("/");
        else if (result == -1)
            return generate301Redirect("/register?error=userexists");
        else if (result == -2) 
            return generate301Redirect("/register?error=usermax");
        else 
            return generate301Redirect("/register?error=unknown");
    } else 
        return generate301Redirect("/register?error=");
    
}

std::string WebServer::messagesPost(std::string data, User* user)
{
    std::regex messageRegex("message=([^&]*)");
    std::smatch match;
    if (std::regex_search(data, match, messageRegex)) {
        std::string messageContent = match[1];
        sendMessage(user, messageContent);
        return generate301Redirect("/");
    } else
        return boilerplate("<html><body><h1>400 Bad Request</h1></body></html>", 400);
}