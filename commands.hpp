/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** commands
*/

#ifndef COMMANDS_H
    #define COMMANDS_H

#include <string>
#include <unistd.h>
#include <sys/socket.h>

class Command {
protected:
    int clientSock;
    bool& isAuthenticated;
    std::string& currentDirectory;

public:
    Command(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : clientSock(clientSock), isAuthenticated(isAuthenticated), currentDirectory(currentDirectory) {}

    virtual ~Command() = default;
    virtual void execute(const std::string& args) = 0;

protected:
    void sendResponse(const std::string& response) {
        send(clientSock, response.c_str(), response.size(), 0);
    }
};

class PWDCommand : public Command {
public:
    PWDCommand(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : Command(clientSock, isAuthenticated, currentDirectory) {}

    void execute(const std::string& args) override;
};

class USERCommand : public Command {
public:
    USERCommand(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : Command(clientSock, isAuthenticated, currentDirectory) {}

    void execute(const std::string& args) override;
};

class PASSCommand : public Command {
public:
    PASSCommand(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : Command(clientSock, isAuthenticated, currentDirectory) {}

    void execute(const std::string& args) override;
};

class QUITCommand : public Command {
public:
    QUITCommand(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : Command(clientSock, isAuthenticated, currentDirectory) {}

    void execute(const std::string& args) override;
};

class CWDCommand : public Command {
public:
    CWDCommand(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : Command(clientSock, isAuthenticated, currentDirectory) {}

    void execute(const std::string& args) override;
};

class LISTCommand : public Command {
public:
    LISTCommand(int clientSock, bool& isAuthenticated, std::string& currentDirectory)
            : Command(clientSock, isAuthenticated, currentDirectory) {}

    void execute(const std::string& args) override;
};

#endif
