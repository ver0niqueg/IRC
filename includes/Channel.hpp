#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string> 
#include <map>    
#include <set>  

class Client;

class Channel
{
    public:
        Channel(const std::string& name);
        ~Channel();

        bool addUser(Client* client, const std::string& key = "");
        bool removeUser(Client* client);
        bool isMember(Client* client) const;

        bool addOperator(Client* client);
        bool removeOperator(Client* client);
        bool isOperator(Client* client) const;

        void setMode(char mode, bool enabled, Client* setter = NULL,
                     const std::string& param = ""); 
        bool getMode(char mode) const;

        void setKey(const std::string& key);
        std::string getKey() const;
        void removeKey();
        void setLimit(int limit);
        int getLimit() const;
        void removeLimit();

        void setTopic(const std::string& topic, Client* setter);
        std::string getTopic() const;

        bool invite(Client* operatorClient, Client* targetClient);
        bool kick(Client* operatorClient, Client* targetClient,
                  const std::string& reason = ""); 

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
        std::map<char, bool> _modes;
        std::string _key;
        int _limit;
};

#endif
