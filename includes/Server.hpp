#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>        
#include <poll.h>

class Client;
class Channel;
class CommandHandler;

class Server
{
	private:
		int					_port;
		std::string			_password;
		std::string			_serverName;
		int					_serverSocket;

		std::map<int, Client*>	_clients;
		std::map<std::string, Channel*>	_channels;
		std::vector<struct pollfd>	_pollFds;

		CommandHandler*		_commandHandler;

		bool				_isrunning;
		void				_initSocket();
		void				_setNonBlocking(int fd);
		void				_acceptNewConnection();
		void				_readClientData(int fd);
		void				_sendPendingData(int fd);
		void				_unsetPollOut(int fd);
		void				_removePollFd(int fd);
		void				_disconnectClient(int fd);
		void				_sendMsgToClient(int fd, const std::string& message);

		Server(const Server& other);
		Server& operator=(const Server& other);

	public:
		Server(int port, const std::string& password);
		~Server();

		void				run();
		void				shutdown();
		void				displayStats() const;

		int					getPort() const;
		const std::string&	getPassword() const;
		const std::string&	getServerName() const;

		void				_setPollOut(int fd);

		Client*				getClient(int fd);
		Client*				getClientByNick(const std::string& nickname);
		void				removeClient(int fd);

		Channel*			getChannel(const std::string& name);
		Channel*			createChannel(const std::string& name);
		void				removeChannel(const std::string& name);

		void				broadcastToChannel(const std::string& channelName,
										   	const std::string& message,
										   	int excludeFd = -1);
};

#endif
