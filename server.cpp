#include "server.hpp"
#include "commands.hpp"
#include "utils.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <arpa/inet.h>

FTPServer::FTPServer(int port, const std::string& homeDir) : port(port), homeDir(homeDir), isAuthenticated(false), dataSock(-1) {
    setupServer();
    initializeCommandMap();
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
    sendResponse(clientSock, "220 Welcome to MyFTP Server\r\n");
}

void FTPServer::handleClient(int clientSock) {
    while (true) {
        std::string command = receiveCommand(clientSock);
        if (command.empty()) {
            break;
        }
        processCommand(clientSock, command);
    }
}

void FTPServer::initializeCommandMap() {
    commandMap["USER"] = [this](int clientSock, const std::string& args) { handleUSER(clientSock, args, currentUser); };
    commandMap["PASS"] = [this](int clientSock, const std::string& args) { handlePASS(clientSock, args, currentUser, isAuthenticated); };
    commandMap["QUIT"] = [this](int clientSock, const std::string&) { handleQUIT(clientSock); };
    commandMap["PWD"] = [this](int clientSock, const std::string&) { handlePWD(clientSock, isAuthenticated); };
    commandMap["CWD"] = [this](int clientSock, const std::string& args) { handleCWD(clientSock, args, isAuthenticated); };
    commandMap["LIST"] = [this](int clientSock, const std::string&) { handleLIST(clientSock, isAuthenticated); };
    commandMap["NOOP"] = [this](int clientSock, const std::string&) { handleNOOP(clientSock); };
    commandMap["HELP"] = [this](int clientSock, const std::string&) { handleHELP(clientSock); };
    commandMap["PASV"] = [this](int clientSock, const std::string&) { handlePASV(clientSock, dataSock); };
    commandMap["PORT"] = [this](int clientSock, const std::string& args) { handlePORT(clientSock, args, dataSock); };
    commandMap["RETR"] = [this](int clientSock, const std::string& args) { handleRETR(clientSock, args, isAuthenticated, dataSock); };
    commandMap["STOR"] = [this](int clientSock, const std::string& args) { handleSTOR(clientSock, args, isAuthenticated, dataSock); };
    commandMap["DELE"] = [this](int clientSock, const std::string& args) { handleDELE(clientSock, args, isAuthenticated); };
}

void FTPServer::processCommand(int clientSock, const std::string& command) {
    if (command.empty() || command.find_first_not_of(' ') == std::string::npos) {
        sendResponse(clientSock, "500 Syntax error, command unrecognized.\r\n");
        return;
    }

    std::string cmd;
    std::string args;
    size_t spacePos = command.find(' ');

    if (spacePos != std::string::npos) {
        cmd = command.substr(0, spacePos);
        args = command.substr(spacePos + 1);
    } else {
        cmd = command;
    }

    auto it = commandMap.find(cmd);
    if (it != commandMap.end()) {
        it->second(clientSock, args);
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