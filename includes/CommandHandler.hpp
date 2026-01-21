#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "NumericReplies.hpp" 
#include <string>             
#include <vector>             
#include <map>                

class Server;
class Client; 

class CommandHandler
{
    public:
        CommandHandler(Server *server);

        typedef void (CommandHandler::*CommandHandlerFunction)(Client* client, const std::vector<std::string>&);

        void processCommand(Client* client, const std::string &input);

    private:
        Server *_server;

        std::map<std::string, CommandHandlerFunction> _commandMap;

        void _initCommandMap();

        void _parseInput(const std::string &input, std::string &command, std::vector<std::string> &params);

        // authorisation commands
        void cmdPass(Client* client, const std::vector<std::string> &params);
        void cmdNick(Client* client, const std::vector<std::string> &params);
        void cmdUser(Client* client, const std::vector<std::string> &params);

        // channel commands
        void cmdJoin(Client* client, const std::vector<std::string> &params);
        void cmdPart(Client* client, const std::vector<std::string> &params);
        void cmdNames(Client* client, const std::vector<std::string> &params);

        // message commands
        void cmdPrivmsg(Client* client, const std::vector<std::string> &params);
        void cmdNotice(Client* client, const std::vector<std::string> &params);

        // operator commands
        void cmdKick(Client* client, const std::vector<std::string> &params);
        void cmdInvite(Client* client, const std::vector<std::string> &params);
        void cmdTopic(Client* client, const std::vector<std::string> &params);
        void cmdMode(Client* client, const std::vector<std::string> &params);

        // utils commands
        void cmdQuit(Client* client, const std::vector<std::string> &params);
        void cmdPing(Client* client, const std::vector<std::string> &params);

        // helpers
        void sendWelcomeMsg(Client* client);
        void sendNumericReply(Client* client, const std::string& numeric, const std::string& message);
        bool isValidNickname(const std::string& nickname);
        bool isValidChannelName(const std::string& name);
};

#endif
