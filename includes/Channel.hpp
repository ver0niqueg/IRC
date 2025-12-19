#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <list>
#include <map>
#include <set>

class Client;

class Channel 
{
    public:
        Channel(const std::string& name);
        ~Channel();

        // members + operators
        bool addUser(Client* client, const std::string& key = "");
        bool removeUser(Client* client);
        bool isMember(Client* client) const;
        bool addOperator(Client* client);
        bool removeOperator(Client* client);
        bool isOperator(Client* client) const;

        // modes
        void setMode(char mode, bool enabled, Client* setter = NULL, const std::string& param = "");
        bool getMode(char mode) const;

        // key + limit
        void setKey(const std::string& key);
        std::string getKey() const;
        void removeKey();
        void setLimit(int limit);
        int getLimit() const;
        void removeLimit();

        // topic
        void setTopic(const std::string& topic, Client* setter);
        std::string getTopic() const;

        // cmds
        bool invite(Client* operatorClient, Client* targetClient);
        bool kick(Client* operatorClient, Client* targetClient, const std::string& reason = "");

        // getters
        std::string getName() const;
        std::set<Client*> getMembersList() const;
        std::set<Client*> getOperatorsList() const;
        std::set<Client*> getInvitedList() const;

    private:
        std::string _name;
        std::set<Client*> _membersList;
        std::set<Client*> _operatorsList;
        std::set<Client*> _invitedList;
        std::string _topic;
        std::map<char, bool> _modes; // i, t, k, l, o
        std::string _key;
        int _limit;
};

#endif