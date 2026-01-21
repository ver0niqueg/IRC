#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>        
#include <poll.h>

// forward declarations
class Client;
class Channel;
class CommandHandler;

class Server
{
	private:
		// server basic config
		int					_port;
		std::string			_password;
		std::string			_serverName;

		// listening socket (to accept new connections)
		int					_serverSocket;

		// clients' list/map
		std::map<int, Client*>	_clients;

		// channels' list/map
		std::map<std::string, Channel*>	_channels;

		// sockets' list to check for events
		std::vector<struct pollfd>	_pollFds;

		// Command handler
		CommandHandler*		_commandHandler;

		// server running state (true = is running)
		bool				_isrunning;

		// private methods (internal utilities)
		void				_initSocket();
		void				_setNonBlocking(int fd);
		void				_acceptNewConnection();
		void				_readClientData(int fd);
		void				_sendPendingData(int fd);
		void				_unsetPollOut(int fd);
		void				_removePollFd(int fd);
		void				_disconnectClient(int fd);
		void				_sendMsgToClient(int fd, const std::string& message);

		// avoid copying
		Server(const Server& other);
		Server& operator=(const Server& other);

	public:
		Server(int port, const std::string& password);
		~Server();

		// main public methods
		void				run();
		void				shutdown();
		void				displayStats() const;

		// getters
		int					getPort() const;
		const std::string&	getPassword() const;
		const std::string&	getServerName() const;

		void				_setPollOut(int fd);

		// clients management
		Client*				getClient(int fd);
		Client*				getClientByNick(const std::string& nickname);
		void				removeClient(int fd);

		// channels management
		Channel*			getChannel(const std::string& name);
		Channel*			createChannel(const std::string& name);
		void				removeChannel(const std::string& name);

		// broadcasting
		void				broadcastToChannel(const std::string& channelName,
										   	const std::string& message,
										   	int excludeFd = -1);
};

#endif
