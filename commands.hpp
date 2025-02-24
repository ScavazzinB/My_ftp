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
void handleNOOP(int clientSock);
void handleHELP(int clientSock);
void handlePASV(int clientSock, int& dataSock);
void handlePORT(int clientSock, const std::string& args, int& dataSock);
void handleRETR(int clientSock, const std::string& args, bool isAuthenticated, int dataSock);
void handleSTOR(int clientSock, const std::string& args, bool isAuthenticated, int dataSock);
void handleDELE(int clientSock, const std::string& args, bool isAuthenticated);

#endif
