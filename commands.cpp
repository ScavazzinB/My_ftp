#include "commands.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include "sys/socket.h"
#include <arpa/inet.h>

void handleUSER(int clientSock, const std::string& args, std::string& currentUser) {
    currentUser = trim(args);
    std::string response = "331 User name okay, need password.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handlePASS(int clientSock, const std::string& args, const std::string& currentUser, bool& isAuthenticated) {
    if (currentUser == "anonymous") {
        isAuthenticated = true;
        std::string response = "230 User logged in, proceed.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleQUIT(int clientSock) {
    std::string response = "221 Goodbye.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handlePWD(int clientSock, bool isAuthenticated) {
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::string response = "257 \"" + std::string(cwd) + "\" is the current directory.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "550 Failed to get current directory.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleCWD(int clientSock, const std::string& args, bool isAuthenticated) {
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

void handleLIST(int clientSock, bool isAuthenticated) {
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
        }
        closedir(dir);
        response = "226 Directory send OK.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "550 Failed to list directory.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleNOOP(int clientSock) {
    std::string response = "200 NOOP command successful.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handleHELP(int clientSock) {
    std::string response = "214 The following commands are recognized:\r\n";
    response += "USER PASS CWD CDUP QUIT DELE PWD PASV PORT HELP NOOP RETR STOR LIST\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handlePASV(int clientSock, int& dataSock) {
    struct sockaddr_in dataAddr;
    socklen_t dataAddrLen = sizeof(dataAddr);
    dataSock = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSock < 0) {
        perror("socket");
        return;
    }
    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_addr.s_addr = INADDR_ANY;
    dataAddr.sin_port = 0; // Let the system choose a port

    if (bind(dataSock, (struct sockaddr*)&dataAddr, sizeof(dataAddr)) < 0) {
        perror("bind");
        close(dataSock);
        return;
    }

    if (listen(dataSock, 1) < 0) {
        perror("listen");
        close(dataSock);
        return;
    }

    getsockname(dataSock, (struct sockaddr*)&dataAddr, &dataAddrLen);
    int port = ntohs(dataAddr.sin_port);
    std::string response = "227 Entering Passive Mode (127,0,0,1," + std::to_string(port / 256) + "," + std::to_string(port % 256) + ").\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handlePORT(int clientSock, const std::string& args, int& dataSock) {
    struct sockaddr_in dataAddr;
    int h1, h2, h3, h4, p1, p2;
    sscanf(args.c_str(), "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    std::string ip = std::to_string(h1) + "." + std::to_string(h2) + "." + std::to_string(h3) + "." + std::to_string(h4);
    int port = p1 * 256 + p2;

    dataSock = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSock < 0) {
        perror("socket");
        return;
    }

    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &dataAddr.sin_addr);
    dataAddr.sin_port = htons(port);

    if (connect(dataSock, (struct sockaddr*)&dataAddr, sizeof(dataAddr)) < 0) {
        perror("connect");
        close(dataSock);
        return;
    }

    std::string response = "200 PORT command successful.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handleRETR(int clientSock, const std::string& args, bool isAuthenticated, int dataSock) {
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }

    std::ifstream file(args, std::ios::binary);
    if (!file.is_open()) {
        std::string response = "550 Failed to open file.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }

    std::string response = "150 Opening binary mode data connection.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(dataSock, buffer, file.gcount(), 0);
    }
    if (file.gcount() > 0) {
        send(dataSock, buffer, file.gcount(), 0);
    }

    file.close();
    close(dataSock);

    response = "226 Transfer complete.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handleSTOR(int clientSock, const std::string& args, bool isAuthenticated, int dataSock) {
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }

    std::ofstream file(args, std::ios::binary);
    if (!file.is_open()) {
        std::string response = "550 Failed to open file.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }

    std::string response = "150 Opening binary mode data connection.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);

    char buffer[1024];
    int bytesRead;
    while ((bytesRead = recv(dataSock, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytesRead);
    }

    file.close();
    close(dataSock);

    response = "226 Transfer complete.\r\n";
    send(clientSock, response.c_str(), response.size(), 0);
}

void handleDELE(int clientSock, const std::string& args, bool isAuthenticated) {
    if (!isAuthenticated) {
        std::string response = "530 Not logged in.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }

    if (unlink(args.c_str()) == 0) {
        std::string response = "250 File deleted successfully.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    } else {
        std::string response = "550 Failed to delete file.\r\n";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}