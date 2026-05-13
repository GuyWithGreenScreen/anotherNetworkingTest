#include "networking.h"
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int mn_init_server(struct ServerCTX *ctx, const struct IPPORT_RAW *listen_addr) {

    ctx->err = 0;

    int sockfd              = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)         {perror("init_server: socket creation fail"); ctx->err = 1; return 1;}

    ctx->sockfd                         = sockfd;
    ctx->server_addr.sin_family         = AF_INET;
    ctx->server_addr.sin_port           = htons(listen_addr->port);
    ctx->server_addr.sin_addr.s_addr    = inet_addr(listen_addr->ip);

    int binded              = bind(sockfd, (struct sockaddr*)&ctx->server_addr, sizeof(struct sockaddr_in));

    if (binded == -1)       {perror("init_server: bind error"); ctx->err = 1; return 1;}

    return 0;
}

int mn_start_server(struct ServerCTX *ctx, int backlog) {

    if (ctx->err)   return 1;

    listen(ctx->sockfd, backlog);

    return 0;
}

int mn_server_accept(struct ServerCTX *ctx, struct ClientOBJ *obj) {

    if (ctx->err)  return 1;

    obj->err                = 0;
    
    int new_sock;

    socklen_t addr_size;
    
    addr_size               = sizeof(struct sockaddr_in);

    new_sock                = accept(ctx->sockfd, (struct sockaddr *)&obj->addr, &addr_size);

    if (new_sock < 0)       {perror("server_accept: socket creation fail"); return 1;}

    obj->sock               = new_sock;

    return 0;

}

size_t mn_server_send(struct ClientOBJ *obj, const unsigned char *dat, size_t dat_len) {

    return send(obj->sock, dat, dat_len, 0);
}

size_t mn_server_recv(struct ClientOBJ *obj, unsigned char *buff, size_t n) {

    return recv(obj->sock, buff, n, 0);
}

int mn_server_close(struct ServerCTX *ctx) {

    int cls = close(ctx->sockfd);

    ctx->sockfd = -1;

    return cls;

}

int mn_server_close_client(struct ClientOBJ *obj) {

    int cls = close(obj->sock);

    obj->sock = -1;

    return cls;

}


// CLIENT SIDE

int mn_init_client(struct ClientCTX *ctx) {
    
    //memset(&ctx, 0, sizeof(struct ClientCTX));
    ctx->err = 0;
    
    return 0;
}

int mn_init_server_obj(struct ServerOBJ *obj, const struct IPPORT_RAW *server_addr) {
    
    obj->err                            = 0;

    obj->sockfd                         = -1;

    obj->server_addr.sin_family         = AF_INET;
    obj->server_addr.sin_port           = htons(server_addr->port);
    obj->server_addr.sin_addr.s_addr    = inet_addr(server_addr->ip);

    return 0;

}

int mn_client_bind_server(struct ClientCTX *ctx, struct ServerOBJ *obj) {

    if (ctx->err)                       return 1;
    
    int sockfd                          = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)                     {perror("init_server: socket creation fail"); obj->err = 1; return 1;}

    obj->sockfd                         = sockfd;

    memcpy(&ctx->connection, obj, sizeof(struct ServerOBJ));

    return 0;

}

int mn_client_connect(struct ClientCTX *ctx) {

    if (ctx->err)       return 1;

    if (connect(ctx->connection.sockfd, (struct sockaddr*)&ctx->connection.server_addr, sizeof(struct sockaddr_in)))
            {perror("client_connect: connection fail"); ctx->err = 1; return 1;}

    return 0;

}

size_t mn_client_send(struct ClientCTX *ctx, const unsigned char *dat, size_t dat_len) {

    return send(ctx->connection.sockfd, dat, dat_len, 0);
}

size_t mn_client_recv(struct ClientCTX *ctx, unsigned char *buff, size_t n) {

    return recv(ctx->connection.sockfd, buff, n, 0);
}


int mn_client_close(struct ClientCTX *ctx) {

    int cls = close(ctx->connection.sockfd);
    
    ctx->connection.sockfd = -1;

    return cls;

}
