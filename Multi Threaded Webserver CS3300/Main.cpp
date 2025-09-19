#include <iostream>
#include "WebServer.h"


int main() {

	WebServer* server = new WebServer(8080);

	server->start();
}