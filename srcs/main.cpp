#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

static Server* g_server = NULL;

void signalHandler(int signum)
{
	(void)signum;
	std::cout << "\nReceived shutdown signal..." << std::endl;
	if (g_server != NULL)
		g_server->shutdown();
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}

	int port = std::atoi(argv[1]);
	if (port <= 1024 && port > 0)
		std::cerr << "Warning: Port " << port << " is a privileged port (1-1024)" << std::endl;

	std::string password = argv[2];

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);

	std::cout << PASTEL_VIOLET << "\nIRC server starting..." << DEFAULT << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Password: " << std::string(password.length(), '*') << std::endl;
	std::cout << std::endl;

	try
	{
		Server server(port, password);
		g_server = &server;
		
		std::cout << PASTEL_GREEN << "Server initialized and ready!" << DEFAULT << std::endl;
		std::cout << "Waiting for connections..." << std::endl;
		std::cout << "Press Ctrl+C to stop the server" << std::endl;
		std::cout << std::endl;
		
		server.run();
		
		server.shutdown();
		g_server = NULL;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		g_server = NULL;
		return (1);
	}

	std::cout << PASTEL_RED << "Server stopped" << DEFAULT << std::endl;
	return (0);
}
