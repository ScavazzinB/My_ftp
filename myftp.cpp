/*
** EPITECH PROJECT, 2025
** B-NWP-400-NCE-4-1-myftp-baptiste.scavazzin
** File description:
** myftp
*/

#include "server.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "USAGE: ./myftp <port> <path>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    const char* path = argv[2];

    FTPServer server(port, path);
    server.start();

    return 0;
}
