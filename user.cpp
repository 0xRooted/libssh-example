#include "headers/user.hpp"
#include <cstdarg>

user::user(ssh_session session, ssh_bind ssh)
{
    this->session = session;
    this->ssh = ssh;
}

bool user::auth(const char *username,const char *password)
{
    if (strcmp(username, "root") == 0 && strcmp(password, "root") == 0)
    {
        this->AuthSuccess = true;
        return true;
    }
    return false;
}

bool user::ssh_init()
{
    if (ssh_handle_key_exchange(this->session) != SSH_OK)
        printf("[SSH-ERROR] [KEY EXCHANGE ERROR]: %s\n", ssh_get_error(this->session));
    else
    {
        ssh_message message;
        do
        {
            message = ssh_message_get(this->session);
            if (message != NULL)
            {
                switch (ssh_message_type(message))
                {
                    case SSH_REQUEST_AUTH:
                        if (ssh_message_subtype(message) == SSH_AUTH_METHOD_PASSWORD)
                        {
                            this->username = ssh_message_auth_user(message);
                            this->password = ssh_message_auth_password(message);
                            if (this->auth(
                                this->username.c_str(),
                                this->password.c_str()
                            ))
                                ssh_message_auth_reply_success(message, 0);
                            else
                            {
                                ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
                                ssh_message_reply_default(message);
                            }
                        }
                        else
                        {
                            ssh_message_auth_set_methods(message, SSH_AUTH_METHOD_PASSWORD);
                            ssh_message_reply_default(message);
                        }
                        break;
                    default:
                        ssh_message_reply_default(message);
                        break;
                }
                ssh_message_free(message);
            }
        } while (!this->AuthSuccess);
        do {
            message = ssh_message_get(session);
            if(message){
                switch(ssh_message_type(message)){
                    case SSH_REQUEST_CHANNEL_OPEN:{
                        if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
                            this->channel = ssh_message_channel_request_open_reply_accept(message);
                            break;
                        }
                        break;
                    }
                    default:
                        ssh_message_reply_default(message);
                }
                ssh_message_free(message);
            }
        } while(message && !this->channel);
    }

    this->ssh_len = sizeof(this->addr);
    this->fd = ssh_get_fd(this->session);
    getpeername(ssh_get_fd(session), (struct sockaddr *)&this->addr, &this->ssh_len);
    this->ip = inet_ntoa(this->addr.sin_addr);
    this->port = ntohs(this->addr.sin_port);
    this->buffer = new char[MAX_BUFF];
    memset(this->buffer, 0, MAX_BUFF);

    return this->AuthSuccess;
}

int user::ssh_decide_key(char buf, bool hide) {
    switch (buf) {
    case SSH_KEY_BACKSPACE:
        if (this->buffer_len > 0) {
            this->ssh_printf("\x08 \x08");
            this->buffer[--this->buffer_len] = 0;
        }
        break;

    case SSH_KEY_RESETLINE:
        ssh_printf("\r\n");
        return 1;

    case SSH_KEY_NEWLINE:
        ssh_printf("\r\n");
        return 1;

    case 0x03:
        this->buffer_len = 0;
        memset(this->buffer, 0, MAX_BUFF);

        ssh_printf("^C\r\n");
        return 2;

    default:
        if (!hide)
            ssh_channel_write(this->channel, &buf, 1);
        else
            ssh_channel_write(this->channel, "*", 1);

        this->buffer[this->buffer_len++] = buf;
    }

    return 0;
}

void user::ssh_printf(std::string format, ...) {
    va_list list;
    char *buffer = NULL;

    va_start(list, format);
    int len = vasprintf(&buffer, format.c_str(), list);
    va_end(list);

    ssh_channel_write(this->channel, buffer, len);
    free(buffer);
}

int user::ssh_read(int size, bool hide) {
    char character;

    for (int i = 0; i < size; i++) {
        if ((this->ret = ssh_channel_read(this->channel, &character, 1, 0)) == SSH_ERROR)
            return -1;
        
        if ((this->ret = this->ssh_decide_key(character, hide)))  {
            if (this->ret != 2) return 1;
            return 0;
        }
    }

    return 0;
}


void user::ssh_handler()
{
    if (this->ssh_init())
    {
        printf("[SSH] [SUCCESSFULLY AUTHENTICATED] [USERNAME: %s] [IP: %s] [PORT: %d]\n", this->username.c_str(), this->ip.c_str(), this->port);
        this->ssh_printf(CLEAR_SCREEN);
        this->ssh_printf(TITANBAY);
        while (this->isAlive)
        {
            memset(this->buffer, 0, MAX_BUFF);
            this->buffer_len = 0;
            this->ssh_printf("%s@%s:~$ ", this->username.c_str(), this->ip.c_str());
            if ((this->ssh_read(256, false)) != -1)
            {
                printf("[USERNAME: %s] [SSH] [COMMAND]: %s\n", this->username.c_str(), buffer);
                if (strcmp(buffer, "exit") == 0)
                    break;
                else if (strcmp(buffer, "clear") == 0)
                {
                    this->ssh_printf(CLEAR_SCREEN);
                    this->ssh_printf(TITANBAY);
                }
                else if (strcmp(buffer, "help") == 0)
                {
                    this->ssh_printf(CLEAR_SCREEN);
                    ssh_printf(HELP);
                }
            }
            else
                break;
        }
    }
    else
        printf("[SSH] [FAILED TO AUTHENTICATE] [USERNAME: %s] [IP: %s] [PORT: %d]\n", this->username.c_str(), this->ip.c_str(), this->port);
    
    printf("[SSH] [USER DISCONNECTED] [USERNAME: %s] [IP: %s] [PORT: %d]\n", this->username.c_str(), this->ip.c_str(), this->port);
    this->clean();
}

void user::clean()
{
    ssh_disconnect(this->session);
    ssh_free(this->session);
    delete[] this->buffer;
    this->buffer = NULL;
    this->isAlive = false;
}