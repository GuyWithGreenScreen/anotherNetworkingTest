#ifndef MYNETWORKINGLIB
#define MYNETWORKINGLIB
#include <linux/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

struct ServerCTX {
    unsigned char err;
    struct sockaddr_in server_addr;
    int sockfd;
};

struct ClientOBJ {
    unsigned char err;
    int sock;
    struct sockaddr_in addr;
};

int check_server(struct ServerCTX *ctx);

int init_server(struct ServerCTX *ctx, const char *ip, const unsigned short port);

int start_server(struct ServerCTX *ctx, int backlog);

int server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj);

#endif
