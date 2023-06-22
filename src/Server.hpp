#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"

#define MAX_CLIENTS 5
#define BUFFER_SIZE 10000

#include <vector>

class Server {
    public:
    Server(void);
    Server(std::string const &password, int const &port);
    Server(Server const &ref);
    ~Server(void);
    Server &operator=(Server const &ref);
    void    startServer();

    private:
    void setupSocket();
    void clientEventHandling();
    void serverEventHandling();

    std::string          password;
    int                  port;
    int                  server_fd;
    std::vector<Client>  clients;
    std::vector<Channel> channels;
    struct pollfd        fds[MAX_CLIENTS];
    struct sockaddr_in   address;
};

#endif
