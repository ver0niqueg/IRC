#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>    
#include <cstddef> 

class Client
{
	private:
		int					_clientFd;
		std::string			_ipAddress;
		int					_port;

		std::string			_nickname;
		std::string			_username;
		std::string			_realname;
		std::string			_hostname;

		bool				_authenticated;
		bool				_passwordGiven;
		bool				_registered; 

		std::string			_receiveBuffer;	
		std::string			_sendBuffer;	

		std::set<std::string>	_joinedChannels;

		Client(const Client& other);
		Client& operator=(const Client& other);

	public:
		Client(int fd, const std::string& ipAddress, int port);
		~Client();

		int					getClientFd() const;
		std::string			getIpAddress() const;
		int					getPort() const;
		std::string			getNickname() const;
		std::string			getUsername() const;
		std::string			getRealname() const;
		std::string			getHostname() const;
		bool				isAuthenticated() const;
		bool				isPasswordGiven() const;
		bool				isRegistered() const;
		const std::string&	getReceiveBuffer() const;
		const std::string&	getSendBuffer() const;

		const std::set<std::string>&	getJoinedChannels() const;

		void				setNickname(const std::string& nickname);
		void				setUsername(const std::string& username);
		void				setRealname(const std::string& realname);
		void				setHostname(const std::string& hostname);
		void				setAuthenticated(bool authenticated);
		void				setPasswordGiven(bool given);
		void				setRegistered(bool registered);

		void				appendToReceiveBuffer(const char* data, size_t size);
		bool				extractCommand(std::string& command); 
		void				appendToSendBuffer(const std::string& data);
		void				consumeFromSendBuffer(size_t bytes); 
		void				clearSendBuffer();

		void				joinChannel(const std::string& channelName);
		void				leaveChannel(const std::string& channelName);
		bool				isInChannel(const std::string& channelName) const;

		std::string			getPrefix() const; 
		void				sendMessage(const std::string& message);
};

#endif
