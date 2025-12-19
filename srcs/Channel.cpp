#include "Channel.hpp"
#include <algorithm>
#include <iostream>
#include <cstdlib>

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _key(""), _limit(0)
{
    _modes['i'] = false;
    _modes['t'] = false;
    _modes['k'] = false;
    _modes['l'] = false;
    _modes['o'] = false;
}

Channel::~Channel() {}

bool Channel::addUser(Client* client, const std::string& key) {
    if (_modes['k'] && _key != "" && key != _key)
        return false;
    if (_modes['l'] && _limit > 0 && (int)_membersList.size() >= _limit)
        return false;
    if (_modes['i'] && _invitedList.find(client) == _invitedList.end())
        return false;
    _membersList.insert(client);
    return true;
}

bool Channel::removeUser(Client* client) {
    return _membersList.erase(client) > 0;
}

bool Channel::isMember(Client* client) const {
    return _membersList.find(client) != _membersList.end();
}

bool Channel::addOperator(Client* client) {
    if (!isMember(client))
        return false;
    _operatorsList.insert(client);
    return true;
}

bool Channel::removeOperator(Client* client) {
    return _operatorsList.erase(client) > 0;
}

bool Channel::isOperator(Client* client) const {
    return _operatorsList.find(client) != _operatorsList.end();
}

void Channel::setMode(char mode, bool enabled, Client* setter, const std::string& param) {
    if (_modes.find(mode) == _modes.end())
        return;
        
    if (setter && !isOperator(setter))
        return;
    _modes[mode] = enabled;
    if (mode == 'k') {
        if (enabled && param != "")
            setKey(param);
        else
            removeKey();
    }
    if (mode == 'l') {
        if (enabled && param != "")
            setLimit(atoi(param.c_str()));
        else
            removeLimit();
    }
}

bool Channel::getMode(char mode) const {
    std::map<char, bool>::const_iterator it = _modes.find(mode);
    if (it != _modes.end())
        return it->second;
    return false;
}

void Channel::setKey(const std::string& key) {
    _key = key;
    _modes['k'] = true;
}

std::string Channel::getKey() const {
    return _key;
}

void Channel::removeKey() {
    _key = "";
    _modes['k'] = false;
}

void Channel::setLimit(int limit) {
    _limit = limit;
    _modes['l'] = true;
}

int Channel::getLimit() const {
    return _limit;
}

void Channel::removeLimit() {
    _limit = 0;
    _modes['l'] = false;
}

void Channel::setTopic(const std::string& topic, Client* setter) {
    if (_modes['t'] && setter && !isOperator(setter))
        return;
    _topic = topic;
}

std::string Channel::getTopic() const {
    return _topic;
}

bool Channel::invite(Client* operatorClient, Client* targetClient) {
    if (!isOperator(operatorClient))
        return false;
    _invitedList.insert(targetClient);
    return true;
}

bool Channel::kick(Client* operatorClient, Client* targetClient, const std::string& reason) {
    if (!isOperator(operatorClient) || !isMember(targetClient)) {
        return false;
    }
    (void)reason;
    removeUser(targetClient);
    removeOperator(targetClient);
    return true;
}

std::string Channel::getName() const {
    return _name;
}

std::set<Client*> Channel::getMembersList() const {
    return _membersList;
}

std::set<Client*> Channel::getOperatorsList() const {
    return _operatorsList;
}

std::set<Client*> Channel::getInvitedList() const {
    return _invitedList;
}
