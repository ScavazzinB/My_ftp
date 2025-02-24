/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** commands
*/

#include "commands.hpp"
#include <dirent.h>
#include <iostream>
#include <fstream>

void PWDCommand::execute(const std::string& args) {
    std::cout << "Executing PWDCommand" << std::endl;
    if (!isAuthenticated) {
        sendResponse("530 Not logged in.\r\n");
        return;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        sendResponse("257 \"" + std::string(cwd) + "\" is the current directory.\r\n");
    } else {
        sendResponse("550 Failed to get current directory.\r\n");
    }
}

void USERCommand::execute(const std::string& args) {
    std::cout << "Executing USERCommand with args: " << args << std::endl;
    currentDirectory = args;
    sendResponse("331 User name okay, need password.\r\n");
}

void PASSCommand::execute(const std::string& args) {
    std::cout << "Executing PASSCommand with args: " << args << std::endl;
    if (currentDirectory == "anonymous") {
        isAuthenticated = true;
        sendResponse("230 User logged in, proceed.\r\n");
    } else {
        sendResponse("530 Not logged in.\r\n");
    }
}

void QUITCommand::execute(const std::string& args) {
    std::cout << "Executing QUITCommand" << std::endl;
    sendResponse("221 Goodbye.\r\n");
}

void CWDCommand::execute(const std::string& args) {
    std::cout << "Executing CWDCommand with args: " << args << std::endl;
    if (!isAuthenticated) {
        sendResponse("530 Not logged in.\r\n");
        return;
    }
    if (chdir(args.c_str()) == 0) {
        sendResponse("250 Directory successfully changed.\r\n");
    } else {
        sendResponse("550 Failed to change directory.\r\n");
    }
}

void LISTCommand::execute(const std::string& args) {
    std::cout << "Executing LISTCommand" << std::endl;
    if (!isAuthenticated) {
        sendResponse("530 Not logged in.\r\n");
        return;
    }
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(".")) != nullptr) {
        sendResponse("150 Here comes the directory listing.\r\n");
        while ((ent = readdir(dir)) != nullptr) {
            std::string entry = ent->d_name;
            send(clientSock, entry.c_str(), entry.size(), 0);
        }
        closedir(dir);
        sendResponse("226 Directory send OK.\r\n");
    } else {
        sendResponse("550 Failed to list directory.\r\n");
    }
}
