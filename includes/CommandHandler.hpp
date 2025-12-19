#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "NumericReplies.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

class CommandHandler
{
    public:
        CommandHandler(Server *server);
        
        // typedef for pointer to member functions of CommandHandler
        typedef void (CommandHandler::*CommandHandlerFunction)(Client* client, const std::vector<std::string>&);
        
        void processCommand(Client* client, const std::string &input);
        
    private:
        Server *_server;
        
        // map of commands to their handlers
        std::map<std::string, CommandHandlerFunction> _commandMap;
        
        void _initCommandMap();
        
        void _parseInput(const std::string &input, std::string &command, std::vector<std::string> &params);
        
        // AUTH COMMANDS
        void cmdPass(Client* client, const std::vector<std::string> &params);
        void cmdNick(Client* client, const std::vector<std::string> &params);
        void cmdUser(Client* client, const std::vector<std::string> &params);
        
        // CHANNEL COMMANDS 
        void cmdJoin(Client* client, const std::vector<std::string> &params);
        void cmdPart(Client* client, const std::vector<std::string> &params);
        void cmdNames(Client* client, const std::vector<std::string> &params);
        
        // MESSAGE COMMANDS
        void cmdPrivmsg(Client* client, const std::vector<std::string> &params);
        void cmdNotice(Client* client, const std::vector<std::string> &params);
        
        // OPERATOR COMMANDS
        void cmdKick(Client* client, const std::vector<std::string> &params);
        void cmdInvite(Client* client, const std::vector<std::string> &params);
        void cmdTopic(Client* client, const std::vector<std::string> &params);
        void cmdMode(Client* client, const std::vector<std::string> &params);
        
        // UTILITY COMMANDS
        void cmdQuit(Client* client, const std::vector<std::string> &params);
        void cmdPing(Client* client, const std::vector<std::string> &params);
        
        // HELPERS
        void sendWelcomeMsg(Client* client);
        void sendNumericReply(Client* client, const std::string& numeric, const std::string& message);
        bool isValidNickname(const std::string& nickname);
        bool isValidChannelName(const std::string& name);
};

#endif