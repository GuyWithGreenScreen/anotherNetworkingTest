#ifndef MYNETWORKINGLIB
#define MYNETWORKINGLIB
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

struct IPPORT_RAW {
    char *ip;
    unsigned short port;
};

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
    int socklisten;
    struct ServerOBJ connections[8];
};

struct ClientOBJ {
    unsigned char err;
    int sock;
    struct sockaddr_in addr;
};

// SERVER SIDE

int mn_check_server(struct ServerCTX *ctx);

int mn_init_server(struct ServerCTX *ctx, const char *ip, const unsigned short port);

int mn_start_server(struct ServerCTX *ctx, int backlog);

int mn_server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj);

size_t mn_server_send(struct ClientOBJ *obj, const unsigned char *dat, size_t dat_len);

size_t mn_server_recv(struct ClientOBJ *obj, unsigned char *buff, size_t n);


// CLIENT SIDE

int mn_init_client(struct ClientCTX *ctx, const char *ip );

int mn_init_server_obj(struct ServerOBJ *obj, const char *ip, const unsigned short port);

int mn_client_connect(struct ClientCTX *ctx, struct ServerOBJ *obj);

size_t mn_client_send(struct ServerOBJ *obj, const unsigned char *dat, size_t dat_len);

//size_t mn_client_recv(struct

#endif
