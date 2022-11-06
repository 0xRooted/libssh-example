#include "headers/server.hpp"

int main(void)
{
    system("clear");
    server *srv = new server("0.0.0.0", 333);

    if (srv->setup())
        srv->handler();
    else
        printf("[SSH] [ERROR IN SETUP]\n");
    
    delete srv;
    return EXIT_SUCCESS;
}