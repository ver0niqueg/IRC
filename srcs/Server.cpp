#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include "Colors.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// server constructor
Server::Server(int port, const std::string& password)
	: _port(port),
	  _password(password),
	  _serverName("ft_irc.42.fr"),
	  _serverSocket(-1),
	  _commandHandler(NULL),
	  _isrunning(false)
{
	std::cout << "Server constructor called..." << std::endl;
	if (port <= 0 || port > 65535)
		throw std::runtime_error("Error: Invalid port number");
	if (password.empty())
		throw std::runtime_error("Error: Password cannot be empty");
	_initSocket();
	_commandHandler = new CommandHandler(this);
	std::cout << "Server object constructed successfully" << std::endl;
}

// server destructor
Server::~Server()
{
	std::cout << "Server destructor called..." << std::endl;
	shutdown();
	
	delete _commandHandler;
	_commandHandler = NULL;
	
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first); // close the client socket (fd)
		delete it->second;
	}
	_clients.clear();
	
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();
	
	if (_serverSocket != -1)
	{
		close(_serverSocket);
		std::cout << "Server socket closed" << std::endl;
	}
		std::cout << "Server object destroyed" << std::endl;
}

// server socket init
void Server::_initSocket()
{
	std::cout << "Initializing server socket..." << std::endl;
	
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
	std::cout << "   Socket created (fd: " << _serverSocket << ") " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	

	int opt = 1; // option we want to activate
	// config an option on the socket: here we set SO_REUSEADDR to reuse the port immediately after program exit
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(_serverSocket);
		throw std::runtime_error(std::string("setsockopt() failed: ") + strerror(errno));
	}
	std::cout << "   Socket option set (SO_REUSEADDR) " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	
	struct sockaddr_in serverAddr; // server adress struct
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET; // IPv4
	serverAddr.sin_addr.s_addr = INADDR_ANY; // bind to all interfaces
	serverAddr.sin_port = htons(_port); // set port (network byte order)

	if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		close(_serverSocket);
		throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));
	}
	std::cout << "   Socket bound to 0.0.0.0:" << _port << " " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	
	if (listen(_serverSocket, SOMAXCONN) < 0)
	{
		close(_serverSocket);
		throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
	}
	std::cout << "   Socket listening (backlog: SOMAXCONN) " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	
	_setNonBlocking(_serverSocket);
	std::cout << "   Socket set to non-blocking mode " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	
	struct pollfd serverPollFd; // checking server socket (fd) for events
	serverPollFd.fd = _serverSocket; // server socket fd
	serverPollFd.events = POLLIN; // we want to read incoming connections
	serverPollFd.revents = 0; // no events yet
	_pollFds.push_back(serverPollFd); // add to poll fds list
	std::cout << "   Server socket added to poll set " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	std::cout << std::endl;

	std::cout << "Server socket initialization complete" << std::endl;
}

// set a socket to non-blocking mode => imporant to handle multiple clients
void Server::_setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0); // options flags of the fd
	if (flags == -1)
		throw std::runtime_error(std::string("fcntl(F_GETFL) failed: ") + strerror(errno));
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error(std::string("fcntl(F_SETFL) failed: ") + strerror(errno));
}

int Server::getPort() const
{
	return (_port);
}

const std::string& Server::getPassword() const
{
	return (_password);
}

const std::string& Server::getServerName() const
{
	return (_serverName);
}

void Server::run()
{
	std::cout << PASTEL_VIOLET << "Starting server event loop..." << DEFAULT << std::endl;
	std::cout << std::endl;
	_isrunning = true;
	while (_isrunning)
	{
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);
		if (pollCount == -1)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "poll() error: " << strerror(errno) << std::endl;
			break;
		}
		// for each fd, check for events
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (_pollFds[i].revents == 0)
				continue;
			if (_pollFds[i].fd == _serverSocket) // if it is the server socket
			{
				if (_pollFds[i].revents & POLLIN) // if a client is trying to connect
					_acceptNewConnection();
			}
			else // if it is a client socket
			{
				int clientFd = _pollFds[i].fd;
				
				if (_pollFds[i].revents & POLLHUP) // if a client disconnect
				{
					std::cout << "Client " << clientFd << " hung up (POLLHUP)" << std::endl;
					_disconnectClient(clientFd);
					break;
				}
				if (_pollFds[i].revents & POLLERR) // if a client socket error occurred
				{
					std::cerr << "Socket error on client " << clientFd << " (POLLERR)" << std::endl;
					_disconnectClient(clientFd);
					break;
				}
				if (_pollFds[i].revents & POLLNVAL) // if an invalid fd
				{
					std::cerr << "Invalid fd " << clientFd << " (POLLNVAL)" << std::endl;
					_disconnectClient(clientFd);
					break;
				}
				
				if (_pollFds[i].revents & POLLIN) // if there is data to read from client
					_readClientData(clientFd);
				
				if (_pollFds[i].revents & POLLOUT) // if ready to send data to client
					_sendPendingData(clientFd);
			}
		}
	}
	std::cout << "Server event loop stopped" << std::endl;
}

void Server::shutdown()
{
	if (_isrunning)
	{
		std::cout << "Stopping server..." << std::endl;
		_isrunning = false;
	}
}

void Server::displayStats() const
{
    std::cout << "\n=== Server Statistics ===" << std::endl;
    std::cout << "  Server running   : " << (_isrunning ? "Yes" : "No") << std::endl;
    std::cout << "  Clients connected: " << _clients.size() << std::endl;
    std::cout << "  Channels active  : " << _channels.size() << std::endl;
    std::cout << "  Poll fds         : " << _pollFds.size()
              << " (1 server + " << (_pollFds.size() > 0 ? _pollFds.size() - 1 : 0) << " clients)" << std::endl;
}

Client* Server::getClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
		return (it->second);
	return (NULL);
}


Client* Server::getClientByNick(const std::string& nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second && it->second->getNickname() == nickname)
			return (it->second);
	}
	return (NULL);
}

void Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		close(it->first);
		delete it->second;
		_clients.erase(it);
	} 
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return (it->second);
	return (NULL);
}

Channel* Server::createChannel(const std::string& name)
{
	Channel* existingChannel = getChannel(name);
	if (existingChannel)
	{
		std::cout << "Channel [" << name << "] already exists" << std::endl;
		return (existingChannel);
	}

	Channel* newChannel = new Channel(name);
	_channels[name] = newChannel;
	std::cout << "Channel [" << name << "] created (" 
	          << _channels.size() << " channels active)" << std::endl;
	return (newChannel);
}


void Server::removeChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
	{
		delete it->second;
		_channels.erase(it);
		std::cout << "Channel [" << name << "] removed (" 
		          << _channels.size() << " channels remaining)" << std::endl;
	}
	else
		std::cout << "Channel [" << name << "] not found" << std::endl;
}

// send a message to all clients in a channel, excluding a specific fd if provided
void Server::broadcastToChannel(const std::string& channelName, const std::string& message, int excludeFd)
{
	Channel* channel = getChannel(channelName); // get the channel object
	if (!channel)
	{
		std::cerr << "Channel [" << channelName << "] not found for broadcast" << std::endl;
		return;
	}
	
	std::set<Client*> members = channel->getMembers();
	std::cout << "   Broadcasting to channel [" << channelName << "] with " 
			<< members.size() << " members" << PASTEL_GREEN << " ✓" << DEFAULT << std::endl;
	
	for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it && (*it)->getClientFd() != excludeFd) // if client fd is different from excluded
		{
			(*it)->sendMessage(message); // add the message to the client's send buffer
			_setPollOut((*it)->getClientFd()); // check socket is ready to send data
		}
	}
	std::cout << "   Message broadcasted: " << PASTEL_GREEN << "✓ " << DEFAULT << message.substr(0, 50) 
	          << (message.length() > 50 ? "..." : "") << std::endl; // truncate the message if too long
}

// handle new incoming connectionsvalgrind ./ircserv 6667 <motdepasse>
void Server::_acceptNewConnection()
{
	struct sockaddr_in clientAddr; // IP + port of the connecting client
	socklen_t clientAddrLen = sizeof(clientAddr); // size of the struct
	while (true)
	{
		int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen); // to get a new fd for the socket client
		if (clientFd == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // no more connections to accept
				break;
			if (errno == EINTR) // interrupted, try again
				continue;
			std::cerr << "accept() error: " << strerror(errno) << std::endl;
			break;
		}

		// extract client IP and port + convert to string
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
		int clientPort = ntohs(clientAddr.sin_port);
		
		std::cout << PASTEL_YELLOW << "[CONNECTION] " << DEFAULT << "New connection from [" << clientIP << "]:" << clientPort 
		          << " (fd: " << clientFd << ")" << std::endl;
		// set the client socket to non-blocking mode
		try
		{
			_setNonBlocking(clientFd);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to set client socket non-blocking: " << e.what() << std::endl;
			close(clientFd);
			continue;
		}
		
		// create a new Client object and add it to the clients map
		Client* newClient = new Client(clientFd, clientIP, clientPort);
		_clients[clientFd] = newClient;
		
		// add the new client socket to the poll fds list (to check for events)
		struct pollfd clientPollFd;
		clientPollFd.fd = clientFd;
		clientPollFd.events = POLLIN;
		clientPollFd.revents = 0;
		_pollFds.push_back(clientPollFd);
		
		std::cout << PASTEL_VIOLET << "[INFO] " << DEFAULT << "Client [" << clientFd << "] added to poll set (" 
				<< _clients.size() << " connected)" << std::endl;
	}
}

// handle reading data from a client
void Server::_readClientData(int fd)
{
	char buffer[4096];
	ssize_t bytesRead;
	
	while (true)
	{
		bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0); // recv to read up to 4095 bytes
		
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			std::cout << PASTEL_RED << "[RECV] " << DEFAULT << "Received " << bytesRead << " bytes from client [" << fd << "]" << std::endl;
			
			Client* client = getClient(fd);
			if (client)
			{
				client->appendToReceiveBuffer(buffer, bytesRead); // add data to client's receive buffer
				
				std::string command;
				while (client->extractCommand(command)) // read complete commands from buffer
				{
					if (_commandHandler)
						_commandHandler->processCommand(client, command);
				}
			}
		}
		else if (bytesRead == 0)
		{

			std::cout << PASTEL_VIOLET << "[INFO] " << DEFAULT << "Client [" << fd << "] closed connection" << std::endl;
			_disconnectClient(fd);
			break;
		}
		else
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else if (errno == EINTR)
				continue;
			else
			{
				std::cerr << "recv() error on client [" << fd << "]: " << strerror(errno) << std::endl;
				_disconnectClient(fd);
				break;
			}
		}
	}
}

// handle properly the disconnection of a client (delete client object, remove from lists, close socket)
void Server::_disconnectClient(int fd)
{
	std::cout << PASTEL_YELLOW << "[DISCONNECTION] " << DEFAULT << "Disconnecting client... " << fd << std::endl;
	
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		if (it->second != NULL)
		{
			std::string nickname = it->second->getNickname();
			if (!nickname.empty())
				std::cout << PASTEL_VIOLET << "[INFO] " << DEFAULT << "Client nickname: [" << nickname << "]" << std::endl;
			
			const std::set<std::string>& channels = it->second->getJoinedChannels();
			for (std::set<std::string>::const_iterator chanIt = channels.begin(); chanIt != channels.end(); ++chanIt)
			{
				Channel* chan = getChannel(*chanIt);
				if (chan)
				{
					std::string quitMsg = it->second->getPrefix() + " QUIT :Client disconnected\r\n";
					broadcastToChannel(*chanIt, quitMsg, fd);
					chan->removeUser(it->second);
					chan->removeOperator(it->second);
				}
			}			
			delete it->second; // delete the Client object
		}
		_clients.erase(it); // remove from clients map
		std::cout << "   Client removed from client list " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	}
	else
		std::cout << "   Warning: Client [" << fd << "] not found in client map" << std::endl;
	
	_removePollFd(fd); // clean up poll fds list
	::shutdown(fd, SHUT_RDWR); // shutdown the socket
	
	if (close(fd) == -1)
		std::cerr << "   close() error: " << strerror(errno) << std::endl;
	else
		std::cout << "   Socket closed " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
	
	std::cout << PASTEL_YELLOW << "[DISCONNECTION] " << DEFAULT << "Client [" << fd << "] disconnected (" 
			<< _clients.size() << " remaining)" << std::endl;
}

// send a message immediately to a client via its socket
void Server::_sendMsgToClient(int fd, const std::string& message)
{
	ssize_t bytesSent = send(fd, message.c_str(), message.length(), 0);
	if (bytesSent == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			std::cerr << "Socket buffer full for client [" << fd << "], message should be queued" << std::endl;
		else
		{
			std::cerr << "send() error on client [" << fd << "]: " << strerror(errno) << std::endl;
			_disconnectClient(fd);
		}
	}
	if (bytesSent < (ssize_t)message.length())
		std::cout << PASTEL_GREEN << "[SEND] " << DEFAULT << "Partial send: " << bytesSent << "/" << message.length() << " bytes to client [" << fd << "]" << std::endl;
	else
		std::cout << PASTEL_GREEN << "[SEND] " << DEFAULT << "Sent " << bytesSent << " bytes to client [" << fd << "]" << std::endl;
}

// handle pending data to send to a client
void Server::_sendPendingData(int fd)
{
	Client* client = getClient(fd);
	if (!client)
	{
		std::cerr << "Client [" << fd << "] not found in _sendPendingData" << std::endl;
		return;
	}
	
	const std::string& sendBuffer = client->getSendBuffer();
	if (sendBuffer.empty())
	{
		_unsetPollOut(fd);
		return;
	}
	
	ssize_t bytesSent = send(fd, sendBuffer.c_str(), sendBuffer.length(), 0);
	if (bytesSent > 0)
	{
		std::cout << PASTEL_GREEN << "[SEND] " << DEFAULT << "Sent " << bytesSent << " bytes to client [" << fd << "]" << std::endl;
		client->consumeFromSendBuffer(bytesSent);
		
		if (client->getSendBuffer().empty())
			_unsetPollOut(fd);
	}
	else if (bytesSent == -1)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "send() error on client [" << fd << "]: " << strerror(errno) << std::endl;
			_disconnectClient(fd);
		}
	}
}

// enable POLLOUT event for a client fd
void Server::_setPollOut(int fd)
{
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			if (!(it->events & POLLOUT))
			{
				it->events |= POLLOUT;
				std::cout << "   POLLOUT enabled for fd [" << fd << "]" << PASTEL_GREEN << " ✓ " << DEFAULT << std::endl;
			}
			return;
		}
	}
	std::cerr << "Warning: fd [" << fd << "] not found in _setPollOut" << std::endl;
}

// disable POLLOUT event for a client fd
void Server::_unsetPollOut(int fd)
{
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			if (it->events & POLLOUT)
			{
				it->events &= ~POLLOUT;
				std::cout << "   POLLOUT disabled for fd [" << fd << "]" << PASTEL_GREEN << " ✓ " << DEFAULT << std::endl;
			}
			return;
		}
	}
	std::cerr << "Warning: fd [" << fd << "] not found in _unsetPollOut" << std::endl;
}

// remove a fd from the poll fds list
void Server::_removePollFd(int fd)
{
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			std::cout << "   File descriptor [" << fd << "] removed from poll set " << PASTEL_GREEN << "✓" << DEFAULT << std::endl;
			return;
		}
	}
	std::cerr << "Warning: fd [" << fd << "] not found in poll set" << std::endl;
}
