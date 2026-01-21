#include "CommandHandler.hpp"
#include "Colors.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include <cctype>
#include <iostream>
#include <sstream>
#include <set>

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

// parse an input from a client (for ex: "PRIVMSG #channel :Hello everyone!")
void CommandHandler::_parseInput(const std::string &input, std::string &command,
                                 std::vector<std::string> &params)
{
    params.clear();
    command.clear();

    if (input.empty())
        return;

    std::istringstream iss(input);
    std::string word;

    // extract the command (first word)
    if (!(iss >> command))
        return;

    // uppercase the command for uniformity
    for (size_t i = 0; i < command.length(); ++i)
        command[i] = std::toupper(command[i]);

    // parse parameters
    while (iss >> word)
    {
        // if a parameter starts with ':', everything after is a single parameter (the message)
        if (!word.empty() && word[0] == ':')
        {
            std::string message = word.substr(1);
            std::string restOfLine;

            std::getline(iss, restOfLine);
            // keep the initial space to preserve the message exactly as typed
            message += restOfLine;

            params.push_back(message);
            break;
        }
        else
        {
            params.push_back(word);
        }
    }
}

// handle a raw command line from a client: dispatch to the appropriate handler
void CommandHandler::processCommand(Client* client, const std::string &input)
{
    if (!client || input.empty())
        return;

    std::string command;
    std::vector<std::string> params;

    _parseInput(input, command, params);
    if (command.empty())
        return;

    std::map<std::string, CommandHandlerFunction>::iterator it = _commandMap.find(command);

    if (it == _commandMap.end())
    {
        std::cerr << PASTEL_YELLOW << "[WARN] " << DEFAULT
                  << "Unknown command from client [fd:" << client->getClientFd() << "] "
                  << "'" << command << "'" << std::endl;

        sendNumericReply(client, ERR_UNKNOWNCOMMAND, command + " :Unknown command");
        return;
    }

    std::cout << PASTEL_VIOLET << "[INFO] " << DEFAULT
              << "Command '" << command << "' received from client [fd:" << client->getClientFd() << "] "
              << "(params: " << params.size() << ")"
              << PASTEL_GREEN << " ✓" << DEFAULT << std::endl;

    CommandHandlerFunction handler = it->second;
    (this->*handler)(client, params);
}

// send a numeric IRC reply according to the IRC protocol (RFC-like format)
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
    _server->_setPollOut(client->getClientFd());
}

// send all the mandatory IRC welcome msgs to a client after successful registration
void CommandHandler::sendWelcomeMsg(Client* client)
{
    const std::string serverVersion = "1.0";
    const std::string serverCreation = "This server was created today";
    const std::string serverModes = "o itkol";

    std::string welcomeMsg = ": Welcome to the Internet Relay Network " + client->getPrefix();
    sendNumericReply(client, RPL_WELCOME, welcomeMsg);

    std::string yourHostMsg = ": Your host is " + _server->getServerName() + ", running version " + serverVersion;
    sendNumericReply(client, RPL_YOURHOST, yourHostMsg);

    sendNumericReply(client, RPL_CREATED, ": " + serverCreation);

    std::string myInfoMsg = _server->getServerName() + " " + serverVersion + " " + serverModes;
    sendNumericReply(client, RPL_MYINFO, myInfoMsg);
}

// check if the nickname is valid according to IRC rules
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

// check if the Channel name is valid according to IRC rules
bool CommandHandler::isValidChannelName(const std::string& name)
{
    static const std::string forbiddenChars = " ,\x07";

    if (name.empty() || name.length() > 50)
        return false;

    if (name[0] != '#' && name[0] != '&')
        return false;

    for (size_t i = 1; i < name.length(); ++i)
    {
        char c = name[i];
        if (forbiddenChars.find(c) != std::string::npos)
            return false;
    }
    return true;
}

/*
** Minimalistic fix for your linker errors:
** - cmdQuit and cmdPing must exist somewhere in compiled objects,
**   because _initCommandMap() references them.
** - We keep them here to avoid adding files / touching headers.
*/

// handle client quit command
void CommandHandler::cmdQuit(Client* client, const std::vector<std::string> &params)
{
    std::string reason = "Client quit";
    if (!params.empty())
        reason = params[0];

    std::cout << "   Client " << client->getNickname() << " quit: " << reason
              << PASTEL_GREEN << " ✓" << DEFAULT << std::endl;

    const std::set<std::string>& channels = client->getJoinedChannels();
    for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        Channel* channel = _server->getChannel(*it);
        if (channel)
        {
            std::string quitMsg = client->getPrefix() + " QUIT :" + reason + "\r\n";
            _server->broadcastToChannel(*it, quitMsg, client->getClientFd());
            channel->removeUser(client);
            channel->removeOperator(client);
        }
    }
    _server->removeClient(client->getClientFd());
}

// respond to PING command from client
void CommandHandler::cmdPing(Client* client, const std::vector<std::string> &params)
{
    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PING :Not enough parameters");
        return;
    }

    std::string response = ":" + _server->getServerName()
        + " PONG " + _server->getServerName()
        + " :" + params[0] + "\r\n";

    client->sendMessage(response);
    _server->_setPollOut(client->getClientFd());
}
