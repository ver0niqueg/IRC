/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vgalmich <vgalmich@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 14:09:59 by vgalmich          #+#    #+#             */
/*   Updated: 2025/12/19 16:39:03 by vgalmich         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "NumericReplies.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

class CommandHandler
{
    public:
        CommandHandler(Server *server);
        
        // typedef pour pointeur vers fonctions membres de CommandHandler
        typedef void (CommandHandler::*CommandHandlerFunction)(Client* client, const std::vector<std::string>&);
        
        void processCommand(Client* client, const std::string &input);
        
    private:
        Server *_server;
        
        // Map des commandes vers leurs handlers
        std::map<std::string, CommandHandlerFunction> _commandMap;
        
        // Initialiser la map
        void _initCommandMap();
        
        // Parse une commande IRC brute en commande + params
        void _parseInput(const std::string &input, std::string &command, std::vector<std::string> &params);
        
        // ========== AUTH COMMANDS ==========
        void cmdPass(Client* client, const std::vector<std::string> &params);
        void cmdNick(Client* client, const std::vector<std::string> &params);
        void cmdUser(Client* client, const std::vector<std::string> &params);
        
        // ========== CHANNEL COMMANDS ==========
        void cmdJoin(Client* client, const std::vector<std::string> &params);
        void cmdPart(Client* client, const std::vector<std::string> &params);
        void cmdNames(Client* client, const std::vector<std::string> &params);
        
        // ========== MESSAGE COMMANDS ==========
        void cmdPrivmsg(Client* client, const std::vector<std::string> &params);
        void cmdNotice(Client* client, const std::vector<std::string> &params);
        
        // ========== OPERATOR COMMANDS ==========
        void cmdKick(Client* client, const std::vector<std::string> &params);
        void cmdInvite(Client* client, const std::vector<std::string> &params);
        void cmdTopic(Client* client, const std::vector<std::string> &params);
        void cmdMode(Client* client, const std::vector<std::string> &params);
        
        // Commandes utilitaires
        void cmdQuit(Client* client, const std::vector<std::string> &params);
        void cmdPing(Client* client, const std::vector<std::string> &params);
        
        // Helpers
        void sendWelcomeMsg(Client* client);
        void sendNumericReply(Client* client, const std::string& numeric, const std::string& message);
        bool isValidNickname(const std::string& nickname);
        bool isValidChannelName(const std::string& name);
};