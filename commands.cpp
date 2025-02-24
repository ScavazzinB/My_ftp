/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** commands
*/

#include "commands.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include "sys/socket.h"

void handleUSER(int clientSock, const std::string& args, std::string& currentUser)
{
    currentUser = trim(args);
    std::string response = "331 User name okay, need password.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handlePASS(int clientSock, const std::string& args, const std::string& currentUser, bool& isAuthenticated)
{
    if (currentUser == "anonymous" && args.empty()) {
        isAuthenticated = true;
        std::string response = "230 User logged in, proceed.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleQUIT(int clientSock)
{
    std::string response = "221 Goodbye.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handlePWD(int clientSock, bool isAuthenticated)
{
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::string response = std::string("257 \"") + cwd + "\"\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "550 Failed to get current directory.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleCWD(int clientSock, const std::string& args, bool isAuthenticated)
{
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }
    if (chdir(args.c_str()) == 0) {
        std::string response = "250 Directory successfully changed.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "550 Failed to change directory.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleLIST(int clientSock, bool isAuthenticated)
{
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != nullptr) {
        std::string response = "150 Here comes the directory listing.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        while ((ent = readdir(dir)) != nullptr) {
            std::string entry = ent->d_name;
            send(clientSock, entry.c_str(), entry.size(), 0);
            send(clientSock, "\r\n", 2, 0);
        }
        closedir(dir);
        response = "226 Directory send OK.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "550 Failed to list directory.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}
