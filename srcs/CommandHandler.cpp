#include "CommandHandler.hpp"
#include <cctype>

CommandHandler::CommandHandler(Server *server) : _server(server)
{
    _initCommandMap();
}

// dispatch IRC commands to the right methods
void CommandHandler::_initCommandMap()
{
    _commandMap["PASS"] = &CommandHandler::cmdPass;
    _commandMap["NICK"] = &CommandHandler::cmdNick;
    _commandMap["USER"] = &CommandHandler::cmdUser;
    
    _commandMap["JOIN"] = &CommandHandler::cmdJoin;
    _commandMap["PART"] = &CommandHandler::cmdPart;
    _commandMap["NAMES"] = &CommandHandler::cmdNames;
    
    _commandMap["PRIVMSG"] = &CommandHandler::cmdPrivmsg;
    _commandMap["NOTICE"] = &CommandHandler::cmdNotice;
    
    _commandMap["KICK"] = &CommandHandler::cmdKick;
    _commandMap["INVITE"] = &CommandHandler::cmdInvite;
    _commandMap["TOPIC"] = &CommandHandler::cmdTopic;
    _commandMap["MODE"] = &CommandHandler::cmdMode;
    
    _commandMap["QUIT"] = &CommandHandler::cmdQuit;
    _commandMap["PING"] = &CommandHandler::cmdPing;
}

// parses an input from a client (for ex: "PRIVMSG #channel :Hello everyone!")
void CommandHandler::_parseInput(const std::string &input, std::string &command, std::vector<std::string> &params)
{
    params.clear();
    command.clear();
    
    if (input.empty())
        return;
    
    std::istringstream iss(input);
    std::string token;
    
    if (!(iss >> command))
        return;
    
    for (size_t i = 0; i < command.length(); ++i)
        command[i] = std::toupper(command[i]);
    
    while (iss >> token)
    {
        if (token[0] == ':')
        {
            std::string msg = token.substr(1);
            std::string rest;
            std::getline(iss, rest);
            msg += rest;
            params.push_back(msg);
            break;
        }
        else
            params.push_back(token);
    }
}

// handles a command from a client
void CommandHandler::processCommand(Client* client, const std::string &input)
{
    if (!client || input.empty())
        return;
    
    std::string command;
    std::vector<std::string> params;
    _parseInput(input, command, params);
    
    if (command.empty())
        return;
    
    std::cout << "Command: " << command << " (params: " << params.size() << ") from client " 
              << client->getClientFd() << std::endl;

    std::map<std::string, CommandHandlerFunction>::iterator it = _commandMap.find(command);
    
    if (it != _commandMap.end())
    {
        CommandHandlerFunction handler = it->second;
        (this->*handler)(client, params);
    }
    else
    {
        sendNumericReply(client, ERR_UNKNOWNCOMMAND, command + " :Unknown command");
    }
}

// sends a numeric IRC reply according to the IRC protocol (RFC)
void CommandHandler::sendNumericReply(Client* client, const std::string& numeric, const std::string& message)
{
    std::stringstream ss;
    ss << ":" << _server->getServerName() << " " << numeric << " ";
    
    if (!client->getNickname().empty())
        ss << client->getNickname();
    else
        ss << "*";
    
    ss << " " << message << "\r\n";
    
    client->sendMessage(ss.str());
}

// sends all the mandatory IRC welcome msgs to a client after a successful connection & registration
void CommandHandler::sendWelcomeMsg(Client* client)
{
    const std::string serverVersion = "1.0";
    const std::string serverCreation = "This server was created today";
    const std::string serverModes = "o itkol";

    std::string welcomeMsg = ":Welcome to the Internet Relay Network " + client->getPrefix();
    sendNumericReply(client, RPL_WELCOME, welcomeMsg);

    std::string yourHostMsg = ":Your host is " + _server->getServerName() + ", running version " + serverVersion;
    sendNumericReply(client, RPL_YOURHOST, yourHostMsg);

    sendNumericReply(client, RPL_CREATED, ":" + serverCreation);

    std::string myInfoMsg = _server->getServerName() + " " + serverVersion + " " + serverModes;
    sendNumericReply(client, RPL_MYINFO, myInfoMsg);
}

bool CommandHandler::isValidNickname(const std::string& nickname)
{
    static const std::string allowedSpecial = "-_[]{}\\|^";

    if (nickname.empty() || nickname.length() > 9)
        return false;

    if (!std::isalpha(nickname[0]))
        return false;

    for (size_t i = 0; i < nickname.length(); ++i)
    {
        char c = nickname[i];
        if (!std::isalnum(c) && allowedSpecial.find(c) == std::string::npos)
            return false;
    }

    return true;
}

bool CommandHandler::isValidChannelName(const std::string& name)
{
    if (name.empty() || name.length() > 50)
        return false;
    
    if (name[0] != '#' && name[0] != '&')
        return false;
    
    for (size_t i = 1; i < name.length(); ++i)
    {
        char c = name[i];
        if (std::isspace(c) || c == ',' || c == '\x07')
            return false;
    }
    
    return true;
}
