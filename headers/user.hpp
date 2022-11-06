#ifndef USER_HPP
#define USER_HPP

#include <libssh/libssh.h>
#include <libssh/server.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define MAX_BUFF          500
#define SSH_KEY_RESETLINE 0xd
#define SSH_KEY_NEWLINE   0xa
#define SSH_KEY_TAB       0x9
#define SSH_KEY_BACKSPACE 0x7f
#define SSH_KEY_SPACE     0x20
#define CLEAR_SCREEN      "\e\143"
#define TITANBAY          "\033[31m╔╦╗╦╔╦╗╔═╗╔╗╔╔╗ ╔═╗╦ ╦\r\n ║ ║ ║ ╠═╣║║║╠╩╗╠═╣╚╦╝\r\n ╩ ╩ ╩ ╩ ╩╝╚╝╚═╝╩ ╩ ╩ \r\n\033[37m"
#define HELP              "\033[31m╦ ╦╔═╗╦  ╔═╗\r\n╠═╣║╣ ║  ╠═╝\r\n╩ ╩╚═╝╩═╝╩  \r\n[1] help\r\n[2] exit\r\n[3] clear\r\n\033[37m"

class user{

    public:
        user(ssh_session session, ssh_bind ssh);
        ssh_session session;
        ssh_bind ssh;
        ssh_channel channel;
        sockaddr_in addr;
        socket_t fd;
        socklen_t ssh_len;
        int port{}, ret{}, buffer_len{};
        std::string username{}, password{}, ip{};
        char *buffer;
        bool AuthSuccess = false, isAlive = true;
        bool auth(const char *username,const char *password);
        bool ssh_init();
        void clean();
        void ssh_handler();
        void ssh_printf(std::string format, ...);
        int ssh_read(int size, bool hide);
        int ssh_decide_key(char buf, bool hide);
};

#endif