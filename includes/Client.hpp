#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>

class Client
{
	private:
		// connection infos
		int					_clientFd;
		std::string			_ipAddress;
		int					_port;
	
		// IRC identification infos
		std::string			_nickname;
		std::string			_username;
		std::string			_realname;
		std::string			_hostname;
	
		// Authentication state
		bool				_authenticated;
		bool				_passwordGiven;
		bool				_registered; // true when NICK + USER are given
	
		// Communication buffers
		std::string			_receiveBuffer;	// Data received waiting to be processed
		std::string			_sendBuffer;	// Data to be sent
	
		// Channels the client belongs to
		std::set<std::string>	_joinedChannels;
	
		// Prevent copying
		Client(const Client& other);
		Client& operator=(const Client& other);

		public:
		// Constructor and destructor
		Client(int fd, const std::string& ipAddress, int port);
		~Client();
	
		// Getters
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
	
		// Setters
		void				setNickname(const std::string& nickname);
		void				setUsername(const std::string& username);
		void				setRealname(const std::string& realname);
		void				setHostname(const std::string& hostname);
		void				setAuthenticated(bool authenticated);
		void				setPasswordGiven(bool given);
		void				setRegistered(bool registered);
	
		// Buffer management
		void				appendToReceiveBuffer(const char* data, size_t size);
		bool				extractCommand(std::string& command); // extracts a complete command (terminated by \r\n)
		void				appendToSendBuffer(const std::string& data);
		void				consumeFromSendBuffer(size_t bytes); // removes the first bytes from the send buffer
		void				clearSendBuffer();
	
		// Channel management
		void				joinChannel(const std::string& channelName);
		void				leaveChannel(const std::string& channelName);
		bool				isInChannel(const std::string& channelName) const;
	
		// Utilities
		std::string			getPrefix() const; // returns the IRC prefix (:nickname!username@hostname)
		void				sendMessage(const std::string& message); // adds to the send buffer
};

#endif
