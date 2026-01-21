#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string> // ajout
#include <map>    // ajout
#include <set>    // ajout

class Client;

class Channel
{
    public:
        Channel(const std::string& name);
        ~Channel();

        // membership
        bool addUser(Client* client, const std::string& key = "");
        bool removeUser(Client* client);
        bool isMember(Client* client) const;

        // operators
        bool addOperator(Client* client);
        bool removeOperator(Client* client);
        bool isOperator(Client* client) const;

        // modes
        void setMode(char mode, bool enabled, Client* setter = NULL,
                     const std::string& param = ""); // CHANGED
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
        bool kick(Client* operatorClient, Client* targetClient,
                  const std::string& reason = ""); // CHANGED

        // getters
        std::string getName() const;
        std::set<Client*> getMembers() const;
        std::set<Client*> getOperators() const;
        std::set<std::string> getInvited() const;

    private:
        std::string _name;
        std::set<Client*> _members;
        std::set<Client*> _operators;
        std::set<std::string> _invited;

        std::string _topic;
        std::map<char, bool> _modes; // i, t, k, l, o
        std::string _key;
        int _limit;
};

#endif
