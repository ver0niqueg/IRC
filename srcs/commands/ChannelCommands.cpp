#include "CommandHandler.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Colors.hpp"
#include <sstream>

// JOIN: let a client join one or more channels
void CommandHandler::cmdJoin(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
    {
        sendNumericReply(client, "451", ":You have not registered"); 
        return;
    }

    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
        return;
    }

    std::vector<std::string> channels;
    std::vector<std::string> keys;

    std::istringstream chanStream(params[0]);
    std::string channel;
    while (std::getline(chanStream, channel, ','))
        channels.push_back(channel);

    if (params.size() > 1)
    {
        std::istringstream keyStream(params[1]);
        std::string key;
        while (std::getline(keyStream, key, ','))
            keys.push_back(key);
    }

    for (size_t i = 0; i < channels.size(); ++i)
    {
        const std::string& channelName = channels[i];
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (!isValidChannelName(channelName))
        {
            sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
            continue;
        }

        Channel* chan = _server->getChannel(channelName);
        bool isNewChannel = (chan == NULL);

        if (isNewChannel)
            chan = _server->createChannel(channelName);

        if (!chan)
        {
            sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :Cannot create channel");
            continue;
        }

        if (chan->isMember(client))
            continue;

        if (!isNewChannel)
        {
            if (chan->getMode('i'))
            {
                std::set<std::string> invited = chan->getInvited();
                if (invited.find(client->getNickname()) == invited.end())
                {
                    sendNumericReply(client, ERR_INVITEONLYCHAN,
                        channelName + " :Cannot join channel (+i)");
                    continue;
                }
            }
            if (chan->getMode('l') && chan->getLimit() > 0)
            {
                std::set<Client*> members = chan->getMembers();
                if ((int)members.size() >= chan->getLimit())
                {
                    sendNumericReply(client, ERR_CHANNELISFULL,
                        channelName + " :Cannot join channel (+l)");
                    continue;
                }
            }
            if (chan->getMode('k') && !chan->getKey().empty())
            {
                if (key != chan->getKey())
                {
                    sendNumericReply(client, ERR_BADCHANNELKEY,
                        channelName + " :Cannot join channel (+k)");
                    continue;
                }
            }
        }
        if (!chan->addUser(client, key))
        {
            sendNumericReply(client, ERR_NOSUCHCHANNEL,
                channelName + " :Cannot join channel");
            continue;
        }

        if (isNewChannel)
            chan->addOperator(client);

        client->joinChannel(channelName);

        std::string joinMsg = client->getPrefix() + " JOIN " + channelName + "\r\n";
        _server->broadcastToChannel(channelName, joinMsg, -1);

        std::string topic = chan->getTopic();
        if (!topic.empty())
            sendNumericReply(client, RPL_TOPIC, channelName + " :" + topic);
        else
            sendNumericReply(client, RPL_NOTOPIC, channelName + " :No topic is set");

        std::string namesList;
        const std::set<Client*> members = chan->getMembers();
        for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
        {
            if (!namesList.empty())
                namesList += " ";

            if (chan->isOperator(*it))
                namesList += "@";

            namesList += (*it)->getNickname();
        }

        sendNumericReply(client, RPL_NAMREPLY, "= " + channelName + " :" + namesList);
        sendNumericReply(client, RPL_ENDOFNAMES, channelName + " :End of /NAMES list");
    }
}

// PART: have a client leave one or more channels
void CommandHandler::cmdPart(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;

    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
        return;
    }

    std::vector<std::string> channels;
    std::istringstream chanStream(params[0]);
    std::string channel;
    while (std::getline(chanStream, channel, ','))
        channels.push_back(channel);

    std::string reason = "Leaving";
    if (params.size() > 1)
        reason = params[1];

    for (size_t i = 0; i < channels.size(); ++i)
    {
        const std::string& channelName = channels[i];

        Channel* chan = _server->getChannel(channelName);
        if (!chan)
        {
            sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
            continue;
        }

        if (!chan->isMember(client))
        {
            sendNumericReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
            continue;
        }

        std::string partMsg = client->getPrefix() + " PART " + channelName + " :" + reason + "\r\n";
        _server->broadcastToChannel(channelName, partMsg, -1);

        chan->removeUser(client);
        chan->removeOperator(client);
        client->leaveChannel(channelName);

        if (chan->getMembers().empty())
            _server->removeChannel(channelName);
    }
}

// NAMES: list nicknames present in a channel
void CommandHandler::cmdNames(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;

    if (params.empty())
    {
        sendNumericReply(client, RPL_ENDOFNAMES, "* :End of /NAMES list");
        return;
    }

    const std::string& channelName = params[0];

    Channel* chan = _server->getChannel(channelName);
    if (!chan)
    {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    std::string namesList;
    const std::set<Client*> members = chan->getMembers();
    for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (!namesList.empty())
            namesList += " ";

        if (chan->isOperator(*it))
            namesList += "@";

        namesList += (*it)->getNickname();
    }

    sendNumericReply(client, RPL_NAMREPLY, "= " + channelName + " :" + namesList);
    sendNumericReply(client, RPL_ENDOFNAMES, channelName + " :End of /NAMES list");
}
