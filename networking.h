#ifndef MYNETWORKINGLIB
#define MYNETWORKINGLIB
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define mn_data_block_header 0xDB01

#define mn_data_block_size_limit 16000
#define mn_data_block_count_limit 256


#define mn_func mn_func_name
#define mn_func_set char *mn_func = 
#define mn_err_ret(X) {printf("%s: %s\n", mn_func, X); return 1;}
#define mn_ERR_ret(X) {printf("%s: %s\n", mn_func, X); perror("Error:"); return 1;}

#define mn_mem_init(amount) void *mn_mem_list[amount] = {0}
#define mn_mem_add(ptr)     {for (int mn_i = 0; mn_i < sizeof(mn_mem_list)/8; mn_i++) {if (!mn_mem_list[mn_i]) mn_mem_list[mn_i] = ptr;}}
#define mn_mem_free()       {for (int mn_i = 0; mn_i < sizeof(mn_mem_list)/8; mn_i++) {free(mn_mem_list[mn_i]);}}
#define mn_mem_err_free(X)  {mn_mem_free(); mn_err_ret(X);}
#define mn_mem_ERR_free(X)  {mn_mem_free(); mn_ERR_ret(X);}

#define mn_byte unsigned char

enum mn_data_block_type {
    mn_s8 = 1,
    mn_s16 = 2,
    mn_s32 = 4,
};

typedef struct mn_data_block {
    enum mn_data_block_type type;
    uint8_t sign;
    uint16_t len;
    void *dat;
} mn_data_block, mn_db, mnDB;

typedef struct mn_IPPORT_RAW {
    char *ip;
    unsigned short port;
} mn_IPPORT_RAW, mn_IPPR, mnIPPR, mn_IP;

typedef struct mn_ServerCTX {
    unsigned char err;
    int sockfd;
    struct sockaddr_in server_addr;
} mn_ServerCTX, mn_SCTX, mnSCTX, mn_SC;

typedef struct mn_ServerOBJ {
    unsigned char err;
    int sockfd;
    struct sockaddr_in server_addr;
} mn_ServerOBJ, mn_SOBJ, mnSOBJ, mn_SO;

typedef struct mn_ClientCTX {
    unsigned char err;
    struct mn_ServerOBJ connection;
} mn_ClientCTX, mn_CCTX, mnCCTX, mn_CC;

typedef struct mn_ClientOBJ {
    unsigned char err;
    int sock;
    struct sockaddr_in addr;
} mn_ClientOBJ, mn_COBJ, mnCOBJ, mn_CO;

// SERVER SIDE

int mn_init_server(struct mn_ServerCTX *ctx, const struct mn_IPPORT_RAW *listen_addr);

int mn_start_server(struct mn_ServerCTX *ctx, int backlog);

int mn_server_accept(struct mn_ServerCTX *ctx, struct mn_ClientOBJ *obj);

size_t mn_server_send(struct mn_ClientOBJ *obj, const unsigned char *dat, size_t dat_len);

int mn_server_send_db(struct mn_ClientOBJ *obj, const struct mn_data_block *blocks, size_t blocks_len);

int mn_server_recv_exact(struct mn_ClientOBJ *obj, unsigned char *buff, size_t n);

int mn_server_recv_db(struct mn_ClientOBJ *obj, struct mn_data_block **blocks, void **block_data, size_t *block_amount);

size_t mn_server_recv(struct mn_ClientOBJ *obj, unsigned char *buff, size_t n);

int mn_server_close(struct mn_ServerCTX *ctx);

int mn_server_close_client(struct mn_ClientOBJ *obj);


// CLIENT SIDE

int mn_init_client(struct mn_ClientCTX *ctx);

int mn_init_server_obj(struct mn_ServerOBJ *obj, const struct mn_IPPORT_RAW *server_addr);

int mn_client_bind_server(struct mn_ClientCTX *ctx, struct mn_ServerOBJ *obj);

int mn_client_connect(struct mn_ClientCTX *ctx);

size_t mn_client_send(struct mn_ClientCTX *ctx, const unsigned char *dat, size_t dat_len);

int mn_client_send_db(struct mn_ClientCTX *ctx, const struct mn_data_block *blocks, size_t blocks_len);

int mn_client_recv_exact(struct mn_ClientCTX *ctx, unsigned char *buff, size_t n);

int mn_client_recv_db(struct mn_ClientCTX *ctx, struct mn_data_block **blocks, void **block_data, size_t *block_amount);

size_t mn_client_recv(struct mn_ClientCTX *ctx, unsigned char *buff, size_t n);

int mn_client_close(struct mn_ClientCTX *ctx);

#endif
