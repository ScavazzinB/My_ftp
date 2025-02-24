/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** commands
*/

#ifndef COMMANDS_H
    #define COMMANDS_H

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Command {
protected:
    int clientSock;
    bool& isAuthenticated;
    std::string& currentDirectory;
    int& dataSock;

public:
    Command(int sock, bool& auth, std::string& dir, int& dataSocket)
            : clientSock(sock), isAuthenticated(auth), currentDirectory(dir), dataSock(dataSocket) {}
    virtual ~Command() {}
    virtual void execute(const std::string& args) = 0;

protected:
    void sendResponse(const std::string& response) {
        send(clientSock, response.c_str(), response.length(), 0);
    }
};

class USERCommand : public Command {
public:
    USERCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

class PASSCommand : public Command {
public:
    PASSCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

class QUITCommand : public Command {
public:
    QUITCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute([[maybe_unused]] const std::string& args) override;
};

class PWDCommand : public Command {
public:
    PWDCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute([[maybe_unused]] const std::string& args) override;
};

class CWDCommand : public Command {
public:
    CWDCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

class LISTCommand : public Command {
public:
    LISTCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute([[maybe_unused]] const std::string& args) override;
};

class TYPECommand : public Command {
public:
    TYPECommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

class SYSTCommand : public Command {
public:
    SYSTCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute([[maybe_unused]] const std::string& args) override;
};

class FEATCommand : public Command {
public:
    FEATCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute([[maybe_unused]] const std::string& args) override;
};

class PASVCommand : public Command {
public:
    PASVCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute([[maybe_unused]] const std::string& args) override;
};

class PORTCommand : public Command {
public:
    PORTCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

class RETRCommand : public Command {
public:
    RETRCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

class STORCommand : public Command {
public:
    STORCommand(int sock, bool& auth, std::string& dir, int& dataSocket)
            : Command(sock, auth, dir, dataSocket) {}
    void execute(const std::string& args) override;
};

#endif
