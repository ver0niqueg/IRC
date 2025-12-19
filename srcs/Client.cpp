#include "Client.hpp"
#include <iostream>
#include <sstream>

Client::Client(int fd, const std::string& ipAddress, int port)
	: _clientFd(fd),
	  _ipAddress(ipAddress),
	  _port(port),
	  _nickname(""),
	  _username(""),
	  _realname(""),
	  _hostname(ipAddress),
	  _authenticated(false),
	  _passwordGiven(false),
	  _registered(false),
	  _receiveBuffer(""),
	  _sendBuffer("")
{
	std::cout << "Client object created (fd: " << _clientFd << ", " << _ipAddress << ":" << _port << ")" << std::endl;
}

Client::~Client()
{
	std::cout << "Client object destroyed (fd: " << _clientFd;
	if (!_nickname.empty())
		std::cout << ", nickname: " << _nickname;
	std::cout << ")" << std::endl;
}

int Client::getClientFd() const
{
	return (_clientFd);
}

std::string Client::getIpAddress() const
{
	return (_ipAddress);
}

int Client::getPort() const
{
	return (_port);
}

std::string Client::getNickname() const
{
	return (_nickname);
}

std::string Client::getUsername() const
{
	return (_username);
}

std::string Client::getRealname() const
{
	return (_realname);
}

std::string Client::getHostname() const
{
	return (_hostname);
}

bool Client::isAuthenticated() const
{
	return (_authenticated);
}

bool Client::isPasswordGiven() const
{
	return (_passwordGiven);
}

bool Client::isRegistered() const
{
	return (_registered);
}

const std::string& Client::getReceiveBuffer() const
{
	return (_receiveBuffer);
}

const std::string& Client::getSendBuffer() const
{
	return (_sendBuffer);
}

const std::set<std::string>& Client::getJoinedChannels() const
{
	return (_joinedChannels);
}

void Client::setNickname(const std::string& nickname)
{
	std::cout << "Client " << _clientFd << " nickname set to: " << nickname << std::endl;
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	std::cout << "Client " << _clientFd << " username set to: " << username << std::endl;
	_username = username;
}

void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

void Client::setHostname(const std::string& hostname)
{
	_hostname = hostname;
}

void Client::setAuthenticated(bool authenticated)
{
	_authenticated = authenticated;
	if (authenticated)
		std::cout << "Client " << _clientFd << " authenticated" << std::endl;
}

void Client::setPasswordGiven(bool given)
{
	_passwordGiven = given;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
	if (registered)
		std::cout << "Client " << _clientFd << " (" << _nickname << ") registered" << std::endl;
}

void Client::appendToReceiveBuffer(const char* data, size_t size)
{
	_receiveBuffer.append(data, size);
}

bool Client::extractCommand(std::string& command)
{
	size_t pos = _receiveBuffer.find("\r\n");
	
	if (pos == std::string::npos)
	{
		pos = _receiveBuffer.find('\n');
		if (pos == std::string::npos)
			return (false);
	}
	
	command = _receiveBuffer.substr(0, pos);
	
	if (_receiveBuffer[pos] == '\r' && pos + 1 < _receiveBuffer.length() && _receiveBuffer[pos + 1] == '\n')
		_receiveBuffer.erase(0, pos + 2);
	else
		_receiveBuffer.erase(0, pos + 1);
	
	std::cout << "Command extracted from client " << _clientFd << ": [" << command << "]" << std::endl;
	return (true);
}

void Client::appendToSendBuffer(const std::string& data)
{
	_sendBuffer.append(data);
	std::cout << "Added " << data.length() << " bytes to send buffer for client " << _clientFd 
	          << " (total: " << _sendBuffer.length() << " bytes)" << std::endl;
}

void Client::consumeFromSendBuffer(size_t bytes)
{
	if (bytes >= _sendBuffer.length())
		_sendBuffer.clear();
	else
		_sendBuffer.erase(0, bytes);
}

void Client::clearSendBuffer()
{
	_sendBuffer.clear();
}

void Client::joinChannel(const std::string& channelName)
{
	_joinedChannels.insert(channelName);
	std::cout << "Client " << _nickname << " joined channel " << channelName 
	          << " (total: " << _joinedChannels.size() << " channels)" << std::endl;
}

void Client::leaveChannel(const std::string& channelName)
{
	_joinedChannels.erase(channelName);
	std::cout << "Client " << _nickname << " left channel " << channelName 
	          << " (remaining: " << _joinedChannels.size() << " channels)" << std::endl;
}

bool Client::isInChannel(const std::string& channelName) const
{
	return (_joinedChannels.find(channelName) != _joinedChannels.end());
}

std::string Client::getPrefix() const
{
	std::stringstream ss;
	ss << ":" << _nickname;
	if (!_username.empty())
		ss << "!" << _username;
	if (!_hostname.empty())
		ss << "@" << _hostname;
	return (ss.str());
}

void Client::sendMessage(const std::string& message)
{
	if (message.length() >= 2 && message.substr(message.length() - 2) == "\r\n")
		appendToSendBuffer(message);
	else
		appendToSendBuffer(message + "\r\n");
}
