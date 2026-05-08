#include "networking.h"
#include <linux/in.h>
#include <stdio.h>
#include <sys/socket.h>

int check_server(struct ServerCTX *ctx) {
    if (ctx->err) {printf("Server in error state"); return 1;}
    return 0;
}

int init_server(struct ServerCTX *ctx, const char *ip, const unsigned short port) {

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

int start_server(struct ServerCTX *ctx, int backlog) {

    if (check_server(ctx))  return 1;

    listen(ctx->sockfd, backlog);

    return 0;
}

int server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj) {

    obj->err    = 0;

    if (check_server(ctx))  return 1;

    int new_sock;

    socklen_t addr_size;
    
    addr_size   = sizeof(struct sockaddr_in);

    new_sock    = accept(ctx->sockfd, (struct sockaddr *)&obj->addr, &addr_size);

    if (new_sock < 0)       {perror("server_accept: socket creation fail"); return 1;}

    obj->sock   = new_sock;

    return 0;
}
