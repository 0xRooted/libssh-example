#ifndef SERVER_HPP
#define SERVER_HPP

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "user.hpp"

class JsonList{
    public:
        JsonList();
        ~JsonList();
        std::string name;
        std::string value;
        JsonList *next;
};

class server{

    public:
        ssh_bind ssh;
        std::string host{};
        int32_t port{};
        bool listen = true;
        std::vector< user * > users{};
        server(std::string host, int32_t port);
        bool setup();
        void handler();
};

#endif