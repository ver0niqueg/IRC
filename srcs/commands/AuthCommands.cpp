#include "CommandHandler.hpp"

// validate the client's password
void CommandHandler::cmdPass(Client* client, const std::vector<std::string> &params)
{
    if (client->isRegistered()) 
    {
        sendNumericReply(client, ERR_ALREADYREGISTRED, ":You may not reregister");
        return;
    }

    if (params.empty()) 
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
        return;
    }

    const std::string& password = params[0];

    if (password == _server->getPassword()) 
    {
        if (client->isPasswordGiven()) 
        {
            std::cout << "Client " << client->getClientFd() << " tried to resend PASS (already accepted)" << std::endl;
            return;
        }
        client->setPasswordGiven(true);
        std::cout << PASTEL_VIOLET << "[INFO] " << DEFAULT << "Client " << client->getClientFd() << " provided correct password" << std::endl;
    } 
    else 
    {
        sendNumericReply(client, ERR_PASSWDMISMATCH, ":Password incorrect");
        std::cout << PASTEL_VIOLET << "[INFO] " << DEFAULT << "Client " << client->getClientFd() << " provided wrong password" << std::endl;
    }
}

// validate or update a client's nickname
void CommandHandler::cmdNick(Client* client, const std::vector<std::string> &params)
{
    if (params.empty()) 
    {
        sendNumericReply(client, ERR_NONICKNAMEGIVEN, ":No nickname given");
        return;
    }

    const std::string& newNick = params[0];

    if (!isValidNickname(newNick)) {
        sendNumericReply(client, ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
        return;
    }

    Client* existingClient = _server->getClientByNick(newNick);
    if (existingClient && existingClient != client) 
    {
        sendNumericReply(client, ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
        return;
    }

    std::string oldNick = client->getNickname();

    if (client->isRegistered() && !oldNick.empty()) 
    {
        const std::set<std::string>& channels = client->getJoinedChannels();
        for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it) 
        {
            Channel* channel = _server->getChannel(*it);
            if (channel) 
            {
                std::string nickChangeMsg = client->getPrefix() + " NICK :" + newNick + "\r\n";
                _server->broadcastToChannel(*it, nickChangeMsg, -1);
            }
        }
        std::cout << "Client " << oldNick << " changed nickname to " << newNick << std::endl;
    }

    client->setNickname(newNick);

    if (!client->getUsername().empty() && client->isPasswordGiven() && !client->isRegistered()) {
        client->setRegistered(true);
        sendWelcomeMsg(client);
    }
}

// register a new client with username and realname
void CommandHandler::cmdUser(Client* client, const std::vector<std::string> &params)
{
    if (client->isRegistered())
    {
        sendNumericReply(client, ERR_ALREADYREGISTRED, ":You may not reregister");
        return;
    }
    
    if (params.size() < 4)
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
        return;
    }
    
    if (!client->isPasswordGiven())
    {
        sendNumericReply(client, ERR_PASSWDMISMATCH, ":You must send PASS first");
        return;
    }
    
    const std::string& username = params[0];
    const std::string& realname = params[3];
    
    client->setUsername(username);
    client->setRealname(realname);
    
    if (!client->getNickname().empty())
    {
        client->setRegistered(true);
        sendWelcomeMsg(client);
    }
}

// handle client quit command
void CommandHandler::cmdQuit(Client* client, const std::vector<std::string> &params)
{
    std::string reason = "Client quit";
    if (!params.empty())
        reason = params[0];
    
    std::cout << "   Client " << client->getNickname() << " quit: " << reason << PASTEL_GREEN << " ✓" << DEFAULT << std::endl;
    
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
}

// respond to PING command from client
void CommandHandler::cmdPing(Client* client, const std::vector<std::string> &params)
{
    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PING :Not enough parameters");
        return;
    }
    
    std::string response = ":" + _server->getServerName() + " PONG " + _server->getServerName() + " :" + params[0] + "\r\n";
    client->sendMessage(response);
}
