/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** commands
*/

#ifndef COMMANDS_H
    #define COMMANDS_H

#include <string>

void handleUSER(int clientSock, const std::string& args, std::string& currentUser);
void handlePASS(int clientSock, const std::string& args, const std::string& currentUser, bool& isAuthenticated);
void handleQUIT(int clientSock);
void handlePWD(int clientSock, bool isAuthenticated);
void handleCWD(int clientSock, const std::string& args, bool isAuthenticated);
void handleLIST(int clientSock, bool isAuthenticated);

#endif
