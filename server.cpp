#include "headers/server.hpp"
#include <thread>

server::server(std::string host, int port){
    this->host = host;
    this->port = port;
}

bool server::setup()
{
    this->ssh = ssh_bind_new();
    ssh_bind_options_set(
        this->ssh,
        SSH_BIND_OPTIONS_BINDPORT,
        &this->port
    );
    ssh_bind_options_set(
        this->ssh,
        SSH_BIND_OPTIONS_RSAKEY,
        "ssh_keys/rsa.key"
    );
    if (ssh_bind_listen(this->ssh) < 0 )
    {
        printf("[SSH] [ERROR IN LISTENING] %s\n", ssh_get_error(this->ssh));
        return false;
    }   
    return true;
}

void server::handler()
{
    printf("[SSH] [SERVER STARTED] [IP: %s] [PORT: %d]\n", this->host.c_str(), this->port);
    while (true)
    {
        if (this->listen)
        {
            ssh_session session = ssh_new();
            ssh_bind_accept(this->ssh, session);
            user *usr = new user(session, this->ssh);
            users.push_back(usr);
            std::thread(&user::ssh_handler, usr).detach();
        }
    }
}