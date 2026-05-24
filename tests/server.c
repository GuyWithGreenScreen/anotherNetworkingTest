#include <stdint.h>
#include <stdio.h>
#include "../networking.h"

int main() {

    // INIT MYNETWORKINGLIB CLIENT AND SERVER OBJECTS

    struct mn_ServerCTX server;
    struct mn_ClientOBJ client;

    struct mn_IPPORT_RAW listenIP = {"192.168.1.11", 6767};


    // START SERVER

    mn_init_server(&server, &listenIP); // INITIALIZE SERVER CONTEXT

    mn_start_server(&server, 5);        // START SERVER

    mn_server_accept(&server, &client); // ACCEPT A CLIENT


    //
    // RECEIVING DATA BLOCKS
    //


    // INIT DATA BLOCK BUFFER

    mn_db *block_buff;

    size_t block_count;

    // RECIEVE DATA BLOCKS

    if (mn_server_recv_db(&client, &block_buff, &block_count, 0, 0)) return 1;

    // CHECK BLOCK COUNT
    
    printf("Block Count: %lu\n", block_count);


    // PRINT BLOCKS NICELY
    
    for (int i = 0; i < block_count; i++) {
        printf("--\nType: %u\nSign: %u\nLen: %u\nData: ", block_buff[i].type, block_buff[i].sign, 
                block_buff[i].len);

        uint8_t type = block_buff[i].type;
        if (type == mn_s8) {

                printf("%s", ((char *)block_buff[i].dat));
                
        } else {

            for (int j = 0; j < block_buff[i].len; j++) {

                 if (type == mn_s16) {
                
                     printf("%u, ", ((uint16_t *)block_buff[i].dat)[j]);
                
                 } else if (type == mn_s32) {
                    
                    printf("%u", ((uint32_t *)block_buff[i].dat)[j]);
                
                }
            }
        }

        printf("\n");
    }


    //
    // SENDING DATA BLOCKS
    //


    // INIT TEST VALUES

    uint32_t test = 67;

    uint16_t test2[] = {51, 23, 25, 55};

    char *text = "Hello to you too!";


    // INIT DATA BLOCK ARRAY

    struct mn_data_block blocks[] ={(mn_db) {mn_s32, 0, 1, &test},
                                    (mn_db) {mn_s16, 0, sizeof(test2)/sizeof(uint16_t), test2},
                                    (mn_db) {mn_s8, 1, strlen(text)+1, text}};


    // SEND DATA BLOCKS

    mn_server_send_db(&client, blocks, sizeof(blocks)/sizeof(struct mn_data_block));


    // FREE DATA BLOCK BUFFER

    free(block_buff);


    // CLOSE CONNECTION

    mn_server_close_client(&client);

    mn_server_close(&server);

    return 0;
}
