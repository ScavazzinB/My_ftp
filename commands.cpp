/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** commands
*/

#include "commands.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sys/stat.h>
#include <time.h>

void USERCommand::execute(const std::string& args) {
    if (args.empty()) {
        sendResponse("501 Syntax error in parameters or arguments.\r\n");
        return;
    }
    isAuthenticated = false;
    currentDirectory = args;
    sendResponse("331 User name okay, need password.\r\n");
}

void PASSCommand::execute([[maybe_unused]] const std::string& args) {
    if (currentDirectory.empty()) {
        sendResponse("503 Login with USER first.\r\n");
        return;
    }
    if (currentDirectory == "anonymous") {
        isAuthenticated = true;
        sendResponse("230 User logged in, proceed.\r\n");
        return;
    }
    sendResponse("530 Login incorrect.\r\n");
    isAuthenticated = false;
}

void QUITCommand::execute([[maybe_unused]] const std::string& args) {
    sendResponse("221 Goodbye.\r\n");
}

void PWDCommand::execute([[maybe_unused]] const std::string& args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::string response = "257 \"" + std::string(cwd) + "\" is the current directory.\r\n";
        sendResponse(response);
    } else {
        sendResponse("550 Failed to get current directory.\r\n");
    }
}

void CWDCommand::execute(const std::string& args) {
    if (chdir(args.c_str()) == 0) {
        currentDirectory = args;
        sendResponse("250 Directory successfully changed.\r\n");
    } else {
        sendResponse("550 Failed to change directory.\r\n");
    }
}

void LISTCommand::execute([[maybe_unused]] const std::string& args) {
    if (dataSock == -1) {
        sendResponse("425 Use PORT or PASV first.\r\n");
        return;
    }

    DIR* dir = opendir(".");
    if (dir == nullptr) {
        sendResponse("550 Failed to open directory.\r\n");
        return;
    }

    sendResponse("150 Here comes the directory listing.\r\n");

    std::string listing;
    struct dirent* entry;
    struct stat statbuf;

    while ((entry = readdir(dir)) != nullptr) {
        if (stat(entry->d_name, &statbuf) == 0) {
            char timebuf[80];
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&statbuf.st_mtime));

            std::stringstream line;
            line << (S_ISDIR(statbuf.st_mode) ? "d" : "-")
                 << ((statbuf.st_mode & S_IRUSR) ? "r" : "-")
                 << ((statbuf.st_mode & S_IWUSR) ? "w" : "-")
                 << ((statbuf.st_mode & S_IXUSR) ? "x" : "-")
                 << ((statbuf.st_mode & S_IRGRP) ? "r" : "-")
                 << ((statbuf.st_mode & S_IWGRP) ? "w" : "-")
                 << ((statbuf.st_mode & S_IXGRP) ? "x" : "-")
                 << ((statbuf.st_mode & S_IROTH) ? "r" : "-")
                 << ((statbuf.st_mode & S_IWOTH) ? "w" : "-")
                 << ((statbuf.st_mode & S_IXOTH) ? "x" : "-")
                 << " 1 owner group "
                 << std::setw(8) << statbuf.st_size << " "
                 << timebuf << " "
                 << entry->d_name << "\r\n";

            listing += line.str();
        }
    }
    closedir(dir);

    if (send(dataSock, listing.c_str(), listing.length(), 0) < 0) {
        sendResponse("426 Connection closed; transfer aborted.\r\n");
    } else {
        sendResponse("226 Directory send OK.\r\n");
    }

    close(dataSock);
    dataSock = -1;
}

void TYPECommand::execute(const std::string& args) {
    if (args == "I" || args == "A") {
        sendResponse("200 Type set to " + args + ".\r\n");
    } else {
        sendResponse("504 Type not supported.\r\n");
    }
}

void SYSTCommand::execute([[maybe_unused]] const std::string& args) {
    sendResponse("215 UNIX Type: L8\r\n");
}

void FEATCommand::execute([[maybe_unused]] const std::string& args) {
    sendResponse("211-Features:\r\n");
    sendResponse(" PASV\r\n");
    sendResponse(" UTF8\r\n");
    sendResponse("211 End\r\n");
}

void PASVCommand::execute([[maybe_unused]] const std::string& args) {
    if (dataSock != -1) {
        close(dataSock);
    }

    int listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock < 0) {
        sendResponse("425 Can't open data connection.\r\n");
        return;
    }

    int yes = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    struct sockaddr_in dataAddr;
    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_addr.s_addr = INADDR_ANY;
    dataAddr.sin_port = 0;

    if (bind(listenSock, (struct sockaddr*)&dataAddr, sizeof(dataAddr)) < 0) {
        sendResponse("425 Can't open data connection.\r\n");
        close(listenSock);
        return;
    }

    if (listen(listenSock, 1) < 0) {
        sendResponse("425 Can't open data connection.\r\n");
        close(listenSock);
        return;
    }

    socklen_t len = sizeof(dataAddr);
    if (getsockname(listenSock, (struct sockaddr*)&dataAddr, &len) < 0) {
        sendResponse("425 Can't open data connection.\r\n");
        close(listenSock);
        return;
    }

    int port = ntohs(dataAddr.sin_port);
    std::stringstream ss;
    ss << "227 Entering Passive Mode (127,0,0,1,"
       << (port >> 8) << "," << (port & 255) << ").\r\n";

    sendResponse(ss.str());

    dataSock = accept(listenSock, nullptr, nullptr);
    close(listenSock);

    if (dataSock < 0) {
        sendResponse("425 Can't open data connection.\r\n");
        return;
    }
}

void PORTCommand::execute(const std::string& args) {
    std::stringstream ss(args);
    std::string segment;
    std::vector<int> parts;

    while (std::getline(ss, segment, ',')) {
        parts.push_back(std::stoi(segment));
    }

    if (parts.size() != 6) {
        sendResponse("501 Invalid PORT command.\r\n");
        return;
    }

    std::string ip = std::to_string(parts[0]) + "." +
                     std::to_string(parts[1]) + "." +
                     std::to_string(parts[2]) + "." +
                     std::to_string(parts[3]);
    int port = (parts[4] << 8) + parts[5];

    if (dataSock != -1) {
        close(dataSock);
    }
    dataSock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &(addr.sin_addr));

    if (connect(dataSock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        sendResponse("425 Can't open data connection.\r\n");
        close(dataSock);
        dataSock = -1;
        return;
    }

    sendResponse("200 PORT command successful.\r\n");
}

void RETRCommand::execute(const std::string& args) {
    if (dataSock == -1) {
        sendResponse("425 Use PORT or PASV first.\r\n");
        return;
    }

    std::ifstream file(args, std::ios::binary);
    if (!file) {
        sendResponse("550 Failed to open file.\r\n");
        return;
    }

    sendResponse("150 Opening BINARY mode data connection for " + args + "\r\n");

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(dataSock, buffer, file.gcount(), 0);
    }
    if (file.gcount() > 0) {
        send(dataSock, buffer, file.gcount(), 0);
    }

    file.close();
    close(dataSock);
    dataSock = -1;

    sendResponse("226 Transfer complete.\r\n");
}

void STORCommand::execute(const std::string& args) {
    if (dataSock == -1) {
        sendResponse("425 Use PORT or PASV first.\r\n");
        return;
    }

    std::ofstream file(args, std::ios::binary);
    if (!file) {
        sendResponse("550 Failed to create file.\r\n");
        return;
    }

    sendResponse("150 Opening BINARY mode data connection for " + args + "\r\n");

    char buffer[1024];
    int bytesRead;
    while ((bytesRead = recv(dataSock, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytesRead);
    }

    file.close();
    close(dataSock);
    dataSock = -1;

    sendResponse("226 Transfer complete.\r\n");
}
