#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <netinet/in.h>
#include "commands.hpp"

class FTPServer {
private:
    int port;
    std::string homeDir;
    std::string currentDirectory;
    int serverSock;
    struct sockaddr_in serverAddr;
    bool isAuthenticated;
    std::string currentUser;
    int dataSock;
    std::map<std::string, std::unique_ptr<Command>> commands;

public:
    FTPServer(int port, const std::string& homeDir);
    void start();

private:
    void setupServer();
    void initializeCommands(int clientSock);    void acceptClient(std::vector<struct pollfd>& fds);
    void handleClient(int clientSock);
    void processCommand(int clientSock, const std::string& command, const std::string& args);
    std::string receiveCommand(int clientSock);
    void sendResponse(int clientSock, const std::string& response);
};

#endif // SERVER_H