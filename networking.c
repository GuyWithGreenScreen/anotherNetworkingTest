#include "networking.h"

int mn_init_server(struct mn_ServerCTX *ctx, const struct mn_IPPORT_RAW *listen_addr) {

    mn_func_set             "mn_init_server";

    ctx->err = 0;

    int sockfd              = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)         {ctx->err = 1; mn_ERR_ret("Socket Creation Fail")}

    ctx->sockfd                         = sockfd;
    ctx->server_addr.sin_family         = AF_INET;
    ctx->server_addr.sin_port           = htons(listen_addr->port);
    ctx->server_addr.sin_addr.s_addr    = inet_addr(listen_addr->ip);

    int binded              = bind(sockfd, (struct sockaddr*)&ctx->server_addr, sizeof(struct sockaddr_in));

    if (binded == -1)       {ctx->err = 1; mn_ERR_ret("Bind Error");}

    return 0;
}

int mn_start_server(struct mn_ServerCTX *ctx, int backlog) {

    mn_func_set             "mn_start_server";

    if (ctx->err)           mn_err_ret("Server Context has error flag");

    listen(ctx->sockfd, backlog);

    return 0;
}

int mn_server_accept(struct mn_ServerCTX *ctx, struct mn_ClientOBJ *obj) {

    mn_func_set             "mn_server_accept";

    if (ctx->err)           mn_err_ret("Server Context has error flag");

    obj->err                = 0;
    
    int new_sock;

    socklen_t addr_size;
    
    addr_size               = sizeof(struct sockaddr_in);

    new_sock                = accept(ctx->sockfd, (struct sockaddr *)&obj->addr, &addr_size);

    if (new_sock < 0)       mn_ERR_ret("Accept fail");

    obj->sock               = new_sock;

    return 0;

}

size_t mn_server_send(struct mn_ClientOBJ *obj, const unsigned char *dat, size_t dat_len) {

    mn_func_set             "mn_server_send";

    if (obj->err)           mn_err_ret("Client Object has error flag");

    return send(obj->sock, dat, dat_len, 0);

}

int mn_server_send_db(struct mn_ClientOBJ *obj, const struct mn_data_block *blocks, size_t blocks_len) {

    mn_func_set                 "mn_client_send_db";

    if (blocks == NULL || blocks_len == 0)
                                mn_err_ret("Invalid Blocks");

    uint32_t raw_data_len            = 0;

    for (int i = 0; i < blocks_len; i++) {

        struct mn_data_block block 
                                = blocks[i];

        if (block.dat == NULL)  mn_err_ret("Block Data Ptr NULL");

        if (!(block.type == mn_s8 || block.type == mn_s16 || block.type == mn_s32))
                                mn_err_ret("Invalid Block Types");

        raw_data_len            += 1; // 1 Byte Type Header [Sign]-[Type]

        raw_data_len            += 2; // 2 Byte Data Len

        size_t block_len_check  = block.len * block.type;

        if (block_len_check >= UINT16_MAX) 
                                mn_err_ret("Block Size Too Big");

        if ((size_t) raw_data_len + block_len_check >= UINT32_MAX)
                                mn_err_ret("DB Payload Overflow");

        raw_data_len            += block_len_check;

    }

    if (raw_data_len <= 3)      mn_err_ret("Raw Data Empty");


    uint32_t payload_data_len   = 2 /*DB Header*/
                                + 2 /*Block Count*/
                                + 4 /*Payload Len*/
                                + raw_data_len /*Raw Data*/;


    mn_mem_init(1);
    
    mn_byte *payload_data       = malloc(payload_data_len);

    if (payload_data == NULL)   mn_err_ret("Malloc Failure");

    mn_mem_add(payload_data);


    uint16_t data_block_header  = htons(mn_data_block_header);

    uint16_t block_count        = htons(blocks_len);

    uint32_t next_payload_len   = raw_data_len;

    next_payload_len            = htonl(next_payload_len);

    size_t pd_offset            = 0;

    memcpy(payload_data + pd_offset, &data_block_header, 2);
    pd_offset                   += 2;

    memcpy(payload_data + pd_offset, &block_count, 2);
    pd_offset                   += 2;

    memcpy(payload_data + pd_offset, &next_payload_len, 4);
    pd_offset                   += 4;


    for (int i = 0; i < blocks_len; i++) {

        struct mn_data_block block 
                                = blocks[i];


        mn_byte  type_header    = block.type | (block.sign << 7);

        uint16_t block_data_len = block.len * block.type;

        uint16_t format_bd_len  = htons(block_data_len);

        memcpy(payload_data + pd_offset, &type_header, 1);
        pd_offset               += 1;

        memcpy(payload_data + pd_offset, &format_bd_len, 2);
        pd_offset               += 2;

        if (block.type > 1) {

            for (int j = 0; j < block_data_len; j += block.type) {

                if (block.type == mn_s16) {

                    uint16_t dat            = *(uint16_t *)(block.dat+j);

                    dat                     = htons(dat);

                    memcpy(payload_data + pd_offset, &dat, block.type);
                    pd_offset               += block.type;

                } else {
 
                    uint32_t dat            = *(uint32_t *)(block.dat+j);

                    dat                     = htonl(dat);

                    memcpy(payload_data + pd_offset, &dat, block.type);
                    pd_offset               += block.type;
                
                }
                
            }

        } else {
            memcpy(payload_data + pd_offset, block.dat, block_data_len);
            pd_offset                   += block_data_len;
        }

    }

    if (pd_offset != payload_data_len) 
                                {mn_mem_free(); mn_err_ret("Written Output Does Not Match!");}

    if (mn_server_send(obj, payload_data, payload_data_len) != payload_data_len)
                                {mn_mem_free(); mn_err_ret("Sent Data Does not Match Target");}

    mn_mem_free();

    return 0;

}


int mn_server_recv_exact(struct mn_ClientOBJ *obj, unsigned char *buff, size_t n) {

    mn_func_set                     "mn_server_recv_exact";

    size_t bytes_read               = 0;

    while (bytes_read < n) {
        int r = recv(obj->sock, buff + bytes_read, n - bytes_read, 0); 
        if (r == 0)                 mn_err_ret("Connection Closed");
        if (r < 0)                  mn_ERR_ret("Recv Error");
        bytes_read                  += r;
    }


    return 0;
}


int mn_server_recv_db(struct mn_ClientOBJ *obj, struct mn_data_block **blocks, void **block_data, size_t *block_amount) {

    mn_func_set                     "mn_server_send_db";

    mn_byte recv_db_header[8];

    if (mn_server_recv_exact(obj, recv_db_header, 8))
                                    mn_ERR_ret("Recv Exact Error");


    uint16_t data_block_header      = htons(mn_data_block_header);

    if (memcmp(recv_db_header, &data_block_header, 2) != 0)
                                    mn_err_ret("Received data not DB Header");


    uint16_t block_count            = ntohs(*(uint16_t*)(recv_db_header+2));

    uint32_t raw_payload_size       = ntohl(*(uint32_t*)(recv_db_header+4));


    if (raw_payload_size > mn_data_block_size_limit)
                                    mn_err_ret("Data Block Payload Too Big");

    if (block_count > mn_data_block_count_limit)
                                    mn_err_ret("Too many data blocks");

    if (block_count == 0)           mn_err_ret("Zero Blocks detected");

    mn_mem_init(2);

    size_t payload_data_size        = raw_payload_size - (block_count*3);

    mn_byte *payload_data           = malloc(payload_data_size);

    if (payload_data == NULL)        mn_ERR_ret("Malloc Failure");

    mn_mem_add(payload_data);


    struct mn_data_block *data_blocks
                                    = malloc(block_count*sizeof(struct mn_data_block));

    if (data_blocks == NULL)        mn_mem_ERR_free("Malloc Fail");

    mn_mem_add(data_blocks);

    size_t pd_offset = 0;

    for (int i = 0; i < block_count; i++) {
        
        mn_byte block_header[3];

        if (mn_server_recv_exact(obj, block_header, 3))
                                    mn_mem_ERR_free("Recv Exact Error");

        mn_byte type_header         = *block_header;

        uint8_t type                = type_header & 0xF;

        uint16_t data_len           = ntohs(*(uint16_t*)(block_header+1));


        struct mn_data_block block  = (mn_db) {type, (type_header & 0x80) ? 1 : 0, data_len/type, payload_data + pd_offset};

        if (type == mn_s8) {

            if (mn_server_recv_exact(obj, payload_data + pd_offset, data_len))
                                    mn_mem_ERR_free("Recv Exact Error");

        } else if (type == mn_s16) {
            
            for (int j = 0; j < data_len; j+=mn_s16) {
                
                uint16_t data;

                if (mn_server_recv_exact(obj, (mn_byte *)&data, mn_s16))
                                    mn_mem_ERR_free("Recv Exact Error");

                data                = ntohs(data);

                memcpy(payload_data + pd_offset + j, &data, mn_s16);
            
            }

        } else if (type == mn_s32) {

            for (int j = 0; j < data_len; j+=mn_s32) {
                
                uint32_t data;

                if (mn_server_recv_exact(obj, (mn_byte *)&data, mn_s32))
                                    mn_mem_ERR_free("Recv Exact Error");

                data                = ntohl(data);

                memcpy(payload_data + pd_offset + j, &data, mn_s32);
            
            }

        }

        memcpy(data_blocks+i, &block, sizeof(struct mn_data_block));

        pd_offset                   += data_len;

    }

    *block_data                     = payload_data;

    *blocks                         = data_blocks;

    *block_amount                   = block_count;

    return 0;

}

size_t mn_server_recv(struct mn_ClientOBJ *obj, unsigned char *buff, size_t n) {

    mn_func_set                     "mc_server_recv";

    if (obj->err)                   mn_err_ret("Client Object has error flag");

    return recv(obj->sock, buff, n, 0);

}

int mn_server_close(struct mn_ServerCTX *ctx) {

    int cls = close(ctx->sockfd);

    ctx->sockfd = -1;
    
    return cls;

}

int mn_server_close_client(struct mn_ClientOBJ *obj) {

    int cls;

    if ((cls = close(obj->sock)))   obj->err = 1;

    obj->sock = -1;

    return cls;

}


// CLIENT SIDE

int mn_init_client(struct mn_ClientCTX *ctx) {
    
    ctx->err = 0;
    
    return 0;
}

int mn_init_server_obj(struct mn_ServerOBJ *obj, const struct mn_IPPORT_RAW *server_addr) {
    
    obj->err                            = 0;

    obj->sockfd                         = -1;

    obj->server_addr.sin_family         = AF_INET;
    obj->server_addr.sin_port           = htons(server_addr->port);
    obj->server_addr.sin_addr.s_addr    = inet_addr(server_addr->ip);

    return 0;

}

int mn_client_bind_server(struct mn_ClientCTX *ctx, struct mn_ServerOBJ *obj) {

    mn_func_set                         "mn_client_bind_server";

    if (ctx->err)                       mn_err_ret("Client Context has error flag");
    
    int sockfd                          = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)                     {obj->err = 1; mn_ERR_ret("Socket Creation fail")}

    obj->sockfd                         = sockfd;

    memcpy(&ctx->connection, obj, sizeof(struct mn_ServerOBJ));

    return 0;

}

int mn_client_connect(struct mn_ClientCTX *ctx) {

    mn_func_set                         "mn_client_connect";

    if (ctx->err)                       mn_err_ret("Client Context has error flag");

    if (connect(ctx->connection.sockfd, (struct sockaddr*)&ctx->connection.server_addr, sizeof(struct sockaddr_in)))
                                        {ctx->err = 1; mn_ERR_ret("Connection Fail");}

    return 0;

}

size_t mn_client_send(struct mn_ClientCTX *ctx, const unsigned char *dat, size_t dat_len) {

    mn_func_set                         "mn_client_send";

    if (ctx->err)                       mn_err_ret("Client Context has error flag");
    
    return send(ctx->connection.sockfd, dat, dat_len, 0);

}

int mn_client_send_db(struct mn_ClientCTX *ctx, const struct mn_data_block *blocks, size_t blocks_len) {

    mn_func_set                 "mn_client_send_db";

    if (blocks == NULL || blocks_len == 0)
                                mn_err_ret("Invalid Blocks");

    uint32_t raw_data_len            = 0;

    for (int i = 0; i < blocks_len; i++) {

        struct mn_data_block block 
                                = blocks[i];

        if (block.dat == NULL)  mn_err_ret("Block Data Ptr NULL");

        if (!(block.type == mn_s8 || block.type == mn_s16 || block.type == mn_s32))
                                mn_err_ret("Invalid Block Types");

        raw_data_len            += 1; // 1 Byte Type Header [Sign]-[Type]

        raw_data_len            += 2; // 2 Byte Data Len

        size_t block_len_check  = block.len * block.type;

        if (block_len_check >= UINT16_MAX) 
                                mn_err_ret("Block Size Too Big");

        if ((size_t) raw_data_len + block_len_check >= UINT32_MAX)
                                mn_err_ret("DB Payload Overflow");

        raw_data_len            += block_len_check;

    }

    if (raw_data_len <= 3)      mn_err_ret("Raw Data Empty");


    uint32_t payload_data_len   = 2 /*DB Header*/
                                + 2 /*Block Count*/
                                + 4 /*Payload Len*/
                                + raw_data_len /*Raw Data*/;


    mn_mem_init(1);
    
    mn_byte *payload_data       = malloc(payload_data_len);

    if (payload_data == NULL)   mn_err_ret("Malloc Failure");

    mn_mem_add(payload_data);


    uint16_t data_block_header  = htons(mn_data_block_header);

    uint16_t block_count        = htons(blocks_len);

    uint32_t next_payload_len   = raw_data_len;

    next_payload_len            = htonl(next_payload_len);

    size_t pd_offset            = 0;

    memcpy(payload_data + pd_offset, &data_block_header, 2);
    pd_offset                   += 2;

    memcpy(payload_data + pd_offset, &block_count, 2);
    pd_offset                   += 2;

    memcpy(payload_data + pd_offset, &next_payload_len, 4);
    pd_offset                   += 4;


    for (int i = 0; i < blocks_len; i++) {

        struct mn_data_block block 
                                = blocks[i];


        mn_byte  type_header    = block.type | (block.sign << 7);

        uint16_t block_data_len = block.len * block.type;

        uint16_t format_bd_len  = htons(block_data_len);

        memcpy(payload_data + pd_offset, &type_header, 1);
        pd_offset               += 1;

        memcpy(payload_data + pd_offset, &format_bd_len, 2);
        pd_offset               += 2;

        if (block.type > 1) {

            for (int j = 0; j < block_data_len; j += block.type) {

                if (block.type == mn_s16) {

                    uint16_t dat            = *(uint16_t *)(block.dat+j);

                    dat                     = htons(dat);

                    memcpy(payload_data + pd_offset, &dat, block.type);
                    pd_offset               += block.type;

                } else {
 
                    uint32_t dat            = *(uint32_t *)(block.dat+j);

                    dat                     = htonl(dat);

                    memcpy(payload_data + pd_offset, &dat, block.type);
                    pd_offset               += block.type;
                
                }
                
            }

        } else {
            memcpy(payload_data + pd_offset, block.dat, block_data_len);
            pd_offset                   += block_data_len;
        }

    }

    if (pd_offset != payload_data_len) 
                                {mn_mem_free(); mn_err_ret("Written Output Does Not Match!");}

    if (mn_client_send(ctx, payload_data, payload_data_len) != payload_data_len)
                                {mn_mem_free(); mn_err_ret("Sent Data Does not Match Target");}

    mn_mem_free();

    return 0;

}

int mn_client_recv_exact(struct mn_ClientCTX *ctx, unsigned char *buff, size_t n) {

    mn_func_set         "mn_server_recv_exact";

    size_t bytes_read   = 0;

    while (bytes_read < n) {
        int r = recv(ctx->connection.sockfd, buff + bytes_read, n - bytes_read, 0); 
        if (r == 0) mn_err_ret("Connection Closed");
        if (r < 0)  mn_ERR_ret("Recv Error");
        bytes_read += r;
    }


    return 0;
}

int mn_client_recv_db(struct mn_ClientCTX *ctx, struct mn_data_block **blocks, void **block_data, size_t *block_amount) {

    mn_func_set                     "mn_server_send_db";

    mn_byte recv_db_header[8];

    if (mn_client_recv_exact(ctx, recv_db_header, 8))
                                    mn_ERR_ret("Recv Exact Error");


    uint16_t data_block_header      = htons(mn_data_block_header);

    if (memcmp(recv_db_header, &data_block_header, 2) != 0)
                                    mn_err_ret("Received data not DB Header");


    uint16_t block_count            = ntohs(*(uint16_t*)(recv_db_header+2));

    uint32_t raw_payload_size       = ntohl(*(uint32_t*)(recv_db_header+4));


    if (raw_payload_size > mn_data_block_size_limit)
                                    mn_err_ret("Data Block Payload Too Big");

    if (block_count > mn_data_block_count_limit)
                                    mn_err_ret("Too many data blocks");

    if (block_count == 0)           mn_err_ret("Zero Blocks detected");

    mn_mem_init(2);

    size_t payload_data_size        = raw_payload_size - (block_count*3);

    mn_byte *payload_data           = malloc(payload_data_size);

    if (payload_data == NULL)        mn_ERR_ret("Malloc Failure");

    mn_mem_add(payload_data);


    struct mn_data_block *data_blocks
                                    = malloc(block_count*sizeof(struct mn_data_block));

    if (data_blocks == NULL)        mn_mem_ERR_free("Malloc Fail");

    mn_mem_add(data_blocks);

    size_t pd_offset = 0;

    for (int i = 0; i < block_count; i++) {
        
        mn_byte block_header[3];

        if (mn_client_recv_exact(ctx, block_header, 3))
                                    mn_mem_ERR_free("Recv Exact Error");

        mn_byte type_header         = *block_header;

        uint8_t type                = type_header & 0xF;

        uint16_t data_len           = ntohs(*(uint16_t*)(block_header+1));


        struct mn_data_block block  = (mn_db) {type, (type_header & 0x80) ? 1 : 0, data_len/type, payload_data + pd_offset};

        if (type == mn_s8) {

            if (mn_client_recv_exact(ctx, payload_data + pd_offset, data_len))
                                    mn_mem_ERR_free("Recv Exact Error");

        } else if (type == mn_s16) {
            
            for (int j = 0; j < data_len; j+=mn_s16) {
                
                uint16_t data;

                if (mn_client_recv_exact(ctx, (mn_byte *)&data, mn_s16))
                                    mn_mem_ERR_free("Recv Exact Error");

                data                = ntohs(data);

                memcpy(payload_data + pd_offset + j, &data, mn_s16);
            
            }

        } else if (type == mn_s32) {

            for (int j = 0; j < data_len; j+=mn_s32) {
                
                uint32_t data;

                if (mn_client_recv_exact(ctx, (mn_byte *)&data, mn_s32))
                                    mn_mem_ERR_free("Recv Exact Error");

                data                = ntohl(data);

                memcpy(payload_data + pd_offset + j, &data, mn_s32);
            
            }

        }

        memcpy(data_blocks+i, &block, sizeof(struct mn_data_block));

        pd_offset                   += data_len;

    }

    *block_data                     = payload_data;

    *blocks                         = data_blocks;

    *block_amount                   = block_count;

    return 0;

}

size_t mn_client_recv(struct mn_ClientCTX *ctx, unsigned char *buff, size_t n) {

    mn_func_set                         "mn_client_recv";

    if (ctx->err)                       mn_err_ret("Client Context has error flag");
    
    return recv(ctx->connection.sockfd, buff, n, 0);

}


int mn_client_close(struct mn_ClientCTX *ctx) {

    int cls;

    if ((cls = close(ctx->connection.sockfd))) 
                                        ctx->err = 1;
    
    ctx->connection.sockfd = -1;

    return cls;

}
