/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** server
*/

#include "server.hpp"
#include "commands.hpp"
#include "utils.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <arpa/inet.h>

FTPServer::FTPServer(int port, const std::string& homeDir)
        : port(port), homeDir(homeDir), isAuthenticated(false) {
    setupServer();
}

void FTPServer::setupServer() {
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        close(serverSock);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSock, 5) < 0) {
        perror("listen");
        close(serverSock);
        exit(EXIT_FAILURE);
    }
}

void FTPServer::start() {
    std::vector<struct pollfd> fds;
    struct pollfd serverPollFd = {serverSock, POLLIN, 0};
    fds.push_back(serverPollFd);

    while (true) {
        int pollCount = poll(fds.data(), fds.size(), -1);
        if (pollCount < 0) {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == serverSock) {
                    acceptClient(fds);
                } else {
                    handleClient(fds[i].fd);
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    --i;
                }
            }
        }
    }

    close(serverSock);
}

void FTPServer::acceptClient(std::vector<struct pollfd>& fds) {
    int clientSock = accept(serverSock, nullptr, nullptr);
    if (clientSock < 0) {
        perror("accept");
        return;
    }
    struct pollfd clientPollFd = {clientSock, POLLIN, 0};
    fds.push_back(clientPollFd);
}

void FTPServer::handleClient(int clientSock) {
    sendResponse(clientSock, "220 Welcome to MyFTP Server\r\n");

    while (true) {
        std::string command = receiveCommand(clientSock);
        if (command.empty()) {
            break;
        }
        processCommand(clientSock, command);
    }
}

void FTPServer::processCommand(int clientSock, const std::string& command) {
    std::string cmd = command.substr(0, 4);
    std::string args = command.length() > 4 ? command.substr(5) : "";

    if (cmd == "USER") {
        handleUSER(clientSock, args, currentUser);
    } else if (cmd == "PASS") {
        handlePASS(clientSock, args, currentUser, isAuthenticated);
    } else if (cmd == "QUIT") {
        handleQUIT(clientSock);
    } else if (cmd == "PWD ") {
        handlePWD(clientSock, isAuthenticated);
    } else if (cmd == "CWD ") {
        handleCWD(clientSock, args, isAuthenticated);
    } else if (cmd == "LIST") {
        handleLIST(clientSock, isAuthenticated);
    } else {
        sendResponse(clientSock, "502 Command not implemented.\r\n");
    }
}

void FTPServer::sendResponse(int clientSock, const std::string& response) {
    send(clientSock, response.c_str(), response.size(), 0);
}

std::string FTPServer::receiveCommand(int clientSock) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        return "";
    }
    return std::string(buffer);
}
