#ifndef MYNETWORKINGLIB
#define MYNETWORKINGLIB
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct mn_IPPORT_RAW {
    char *ip;
    unsigned short port;
};

struct mn_ServerCTX {
    unsigned char err;
    int sockfd;
    struct sockaddr_in server_addr;
};

struct mn_ServerOBJ {
    unsigned char err;
    int sockfd;
    struct sockaddr_in server_addr;
};

struct mn_ClientCTX {
    unsigned char err;
    struct mn_ServerOBJ connection;
};

struct mn_ClientOBJ {
    unsigned char err;
    int sock;
    struct sockaddr_in addr;
};

// SERVER SIDE

int mn_init_server(struct mn_ServerCTX *ctx, const struct mn_IPPORT_RAW *listen_addr);

int mn_start_server(struct mn_ServerCTX *ctx, int backlog);

int mn_server_accept(struct mn_ServerCTX *ctx, struct mn_ClientOBJ *obj);

size_t mn_server_send(struct mn_ClientOBJ *obj, const unsigned char *dat, size_t dat_len);

size_t mn_server_recv(struct mn_ClientOBJ *obj, unsigned char *buff, size_t n);

int mn_server_close(struct mn_ServerCTX *ctx);

int mn_server_close_client(struct mn_ClientOBJ *obj);


// CLIENT SIDE

int mn_init_client(struct mn_ClientCTX *ctx);

int mn_init_server_obj(struct mn_ServerOBJ *obj, const struct mn_IPPORT_RAW *server_addr);

int mn_client_bind_server(struct mn_ClientCTX *ctx, struct mn_ServerOBJ *obj);

int mn_client_connect(struct mn_ClientCTX *ctx);

size_t mn_client_send(struct mn_ClientCTX *ctx, const unsigned char *dat, size_t dat_len);

size_t mn_client_recv(struct mn_ClientCTX *ctx, unsigned char *buff, size_t n);

int mn_client_close(struct mn_ClientCTX *ctx);

#endif
