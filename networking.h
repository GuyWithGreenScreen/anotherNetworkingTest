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
    int sockfd;
    struct sockaddr_in server_addr;
};

struct ClientCTX {
    unsigned char err;
    struct ServerOBJ connection;
};

struct ClientOBJ {
    unsigned char err;
    int sock;
    struct sockaddr_in addr;
};

// SERVER SIDE

int mn_init_server(struct ServerCTX *ctx, const struct IPPORT_RAW *listen_addr);

int mn_start_server(struct ServerCTX *ctx, int backlog);

int mn_server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj);

size_t mn_server_send(struct ClientOBJ *obj, const unsigned char *dat, size_t dat_len);

size_t mn_server_recv(struct ClientOBJ *obj, unsigned char *buff, size_t n);

int mn_server_close(struct ServerCTX *ctx);

int mn_server_close_client(struct ClientOBJ *obj);


// CLIENT SIDE

int mn_init_client(struct ClientCTX *ctx);

int mn_init_server_obj(struct ServerOBJ *obj, const struct IPPORT_RAW *server_addr);

int mn_client_bind_server(struct ClientCTX *ctx, struct ServerOBJ *obj);

int mn_client_connect(struct ClientCTX *ctx);

size_t mn_client_send(struct ClientCTX *ctx, const unsigned char *dat, size_t dat_len);

size_t mn_client_recv(struct ClientCTX *ctx, unsigned char *buff, size_t n);

int mn_client_close(struct ClientCTX *ctx);

#endif
