/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** server
*/

#ifndef SERVER_H
    #define SERVER_H

    #include <string>
    #include <vector>
    #include <netinet/in.h>


class FTPServer {
public:
    FTPServer(int port, const std::string& homeDir);
    void start();


private:
    void setupServer();
    void acceptClient(std::vector<struct pollfd>& fds);
    void handleClient(int clientSock);
    void processCommand(int clientSock, const std::string& command);
    void sendResponse(int clientSock, const std::string& response);
    std::string receiveCommand(int clientSock);

    int serverSock;
    int port;
    std::string homeDir;
    struct sockaddr_in serverAddr;
    std::string currentUser;
    bool isAuthenticated;
};

#endif
