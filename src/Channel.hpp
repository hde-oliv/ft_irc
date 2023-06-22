#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>

#include "Client.hpp"

#define MAX_CHANNEL_NAME 200
#define CHANNEL_PREFIX "&#"
#define CHANNEL_INVALID_CHARS " ,"
#define USER_CHANNEL_LIMIT 10

#include <vector>

class Channel {
    public:
    Channel(void);
    ~Channel(void);

    private:
    std::string         name;
    std::string         topic;
    std::string         host;
    std::string         password;
    std::vector<Client> operators;
    std::vector<Client> clients;
    unsigned int        modes;
    int                 limit;
};

#endif
