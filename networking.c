#include "networking.h"
#include <linux/in.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>

int mn_check_server(struct ServerCTX *ctx) {
    if (ctx->err) {printf("Server in error state"); return 1;}
    return 0;
}

int mn_init_server(struct ServerCTX *ctx, const char *ip, const unsigned short port) {

    ctx->err = 0;

    int sockfd              = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)         {perror("init_server: socket creation fail"); ctx->err = 1; return 1;}

    ctx->sockfd                         = sockfd;
    ctx->server_addr.sin_family         = AF_INET;
    ctx->server_addr.sin_port           = htons(port);
    ctx->server_addr.sin_addr.s_addr    = inet_addr(ip);

    int binded              = bind(sockfd, (struct sockaddr*)&ctx->server_addr, sizeof(struct sockaddr_in));

    if (binded == -1)       {perror("init_server: bind error"); ctx->err = 1; return 1;}

    return 0;
}

int mn_start_server(struct ServerCTX *ctx, int backlog) {

    if (mn_check_server(ctx))  return 1;

    listen(ctx->sockfd, backlog);

    return 0;
}

int mn_server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj) {

    if (mn_check_server(ctx))  return 1;

    obj->err    = 0;
    
    int new_sock;

    socklen_t addr_size;
    
    addr_size   = sizeof(struct sockaddr_in);

    new_sock    = accept(ctx->sockfd, (struct sockaddr *)&obj->addr, &addr_size);

    if (new_sock < 0)       {perror("server_accept: socket creation fail"); return 1;}

    obj->sock   = new_sock;

    return 0;

}

size_t mn_server_send(struct ClientOBJ *obj, const unsigned char *dat, size_t dat_len) {

    return send(obj->sock, dat, dat_len, 0);
}

size_t mn_server_recv(struct ClientOBJ *obj, unsigned char *buff, size_t n) {

    return recv(obj->sock, buff, n, 0);
}


// CLIENT SIDE

int mn_init_client(struct ClientCTX *ctx) {
    
    ctx->err = 0;

    memset(&ctx->connections, 0, 8);
    
    return 0;
}

int mn_init_server_obj(struct ServerOBJ *obj, const char *ip, const unsigned short port) {
    
    obj->err = 0;

    int sockfd              = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)         {perror("init_server: socket creation fail"); obj->err = 1; return 1;}

    obj->sockfd                         = sockfd;
    obj->server_addr.sin_family         = AF_INET;
    obj->server_addr.sin_port           = htons(port);
    obj->server_addr.sin_addr.s_addr    = inet_addr(ip);

    return 0;

}

int mn_client_connect(struct ClientCTX *ctx, struct ServerOBJ *obj) {

    if (ctx->err)       return 1;

    int connection, ptr;

    // FIND SPACE

    struct ServerOBJ    blank;

    memset(&blank, 0, sizeof(blank));

    for (int i = 0; i < sizeof(ctx->connections); i++) {
        if (memcmp(ctx->connections + i, &blank, sizeof(blank)) == 0) {
            ptr = i;
            goto success;
        }
    }

    printf("Not enough space for connections in client ctx\n");
    return 1;

    success:

    if ((connection = connect(obj->sockfd, (struct sockaddr *)&obj->server_addr, sizeof(struct sockaddr_in))) == -1) 
    {perror("client_connect: Failed to connect to server"); return 1;}

    memcpy(ctx->connections + ptr, obj, sizeof(blank));

    return 0;
}

size_t mn_client_send(struct ServerOBJ *obj, const unsigned char *dat, size_t dat_len) {

    return send(obj->sockfd, dat, dat_len, 0);
}
