#include "server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <arpa/inet.h>

FTPServer::FTPServer(int port, const std::string& homeDir)
        : port(port), homeDir(homeDir), currentDirectory(homeDir),
          isAuthenticated(false), dataSock(-1) {
    setupServer();
    initializeCommands(-1);
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

void FTPServer::initializeCommands(int clientSock) {
    commands["USER"] = std::unique_ptr<USERCommand>(new USERCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["PASS"] = std::unique_ptr<PASSCommand>(new PASSCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["QUIT"] = std::unique_ptr<QUITCommand>(new QUITCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["PWD"] = std::unique_ptr<PWDCommand>(new PWDCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["CWD"] = std::unique_ptr<CWDCommand>(new CWDCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["LIST"] = std::unique_ptr<LISTCommand>(new LISTCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["TYPE"] = std::unique_ptr<TYPECommand>(new TYPECommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["SYST"] = std::unique_ptr<SYSTCommand>(new SYSTCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["FEAT"] = std::unique_ptr<FEATCommand>(new FEATCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
    commands["PASV"] = std::unique_ptr<PASVCommand>(new PASVCommand(clientSock, isAuthenticated, currentDirectory, dataSock));
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
    initializeCommands(clientSock);
}

void FTPServer::handleClient(int clientSock) {
    currentDirectory = homeDir;
    isAuthenticated = false;

    if (chdir(currentDirectory.c_str()) != 0) {
        std::cerr << "Failed to change to home directory" << std::endl;
        return;
    }

    while (true) {
        std::string command = receiveCommand(clientSock);
        if (command.empty()) {
            break;
        }

        size_t spacePos = command.find(' ');
        std::string cmd = (spacePos != std::string::npos) ? command.substr(0, spacePos) : command;
        std::string args = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";

        processCommand(clientSock, cmd, args);
    }
}

void FTPServer::processCommand(int clientSock, const std::string& command, const std::string& args) {
    std::cout << "Processing command: " << command << " with args: " << args << std::endl;

    auto it = commands.find(command);
    if (it != commands.end()) {
        it->second->execute(args);
    } else {
        sendResponse(clientSock, "502 Command not implemented.\r\n");
    }
}

std::string FTPServer::receiveCommand(int clientSock) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        return "";
    }
    std::string command(buffer);
    if (!command.empty() && command[command.length()-1] == '\n') {
        command.erase(command.length()-1);
    }
    if (!command.empty() && command[command.length()-1] == '\r') {
        command.erase(command.length()-1);
    }
    return command;
}

void FTPServer::sendResponse(int clientSock, const std::string& response) {
    send(clientSock, response.c_str(), response.size(), 0);
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
