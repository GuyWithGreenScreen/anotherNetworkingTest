#ifndef MYNETWORKINGLIB
#define MYNETWORKINGLIB
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

struct ServerCTX {
    unsigned char err;
    int sockfd;
    struct sockaddr_in server_addr;
};

struct ServerOBJ {
    unsigned char err;
    struct sockaddr_in server_addr;
    int sockfd;
};

struct ClientCTX {
    unsigned char err;
};

struct ClientOBJ {
    unsigned char err;
    int sock;
    struct sockaddr_in addr;
};

// SERVER SIDE

int check_server(struct ServerCTX *ctx);

int init_server(struct ServerCTX *ctx, const char *ip, const unsigned short port);

int start_server(struct ServerCTX *ctx, int backlog);

int server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj);

int server_send(struct ClientOBJ *obj, const unsigned char *dat, size_t dat_len);


// CLIENT SIDE

int init_client(struct ClientCTX *ctx);

int init_server_obj(struct ServerOBJ *obj, const char *ip, const unsigned short port);

int client_connect(struct ClientCTX *ctx, struct ServerOBJ *obj);

#endif
