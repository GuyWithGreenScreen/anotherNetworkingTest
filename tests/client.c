#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "networking.h"


int main() {

    // INIT MYNETWORKINGLIB CLIENT AND SERVER OBJECTS

    struct mn_ClientCTX client;

    struct mn_ServerOBJ server;
    struct mn_IPPORT_RAW serverIP = {"192.168.1.11", 6767};


    // INITIALIZE CLIENT

    mn_init_client(&client);                // INIT CLIENT CONTEXT

    mn_init_server_obj(&server, &serverIP); // INIT SERVER OBJECT

    mn_client_bind_server(&client, &server);// BIND SERVER OBJECT TO CLIENT CONTEXT

    mn_client_connect(&client);             // CONNECT TO SERVER OBJECT


    //
    // SENDING DATA BLOCKS
    //


    // INIT TEST DATA

    uint32_t test = 262983;

    uint16_t test2[] = {51, 23, 325, 345,12, 123, 124, 2343, 62};

    char *text = "Hello my friend!";


    // INIT DATA BLOCK ARRAY

    struct mn_data_block blocks[] ={// (mn_db) {mn_s32, 0, 1, &test}, // I TURNED THIS OFF FOR TESTING
                                    (mn_db) {mn_s16, 0, sizeof(test2)/sizeof(uint16_t), test2},
                                    (mn_db) {mn_s8, 1, strlen(text)+1, text}};


    // SEND DATA BLOCKS TO SERVER

    mn_client_send_db(&client, blocks, sizeof(blocks)/sizeof(struct mn_data_block));


    //
    // RECEIVING DATA BLOCKS
    //


    // INIT DATA BLOCK BUFFER

    mn_db *block_buff;

    size_t block_count;


    // RECEIVE DATA BLOCKS FROM SERVER

    //                                    *THIS IS THE VALUE >| 
    if (mn_client_recv_db(&client, &block_buff, &block_count, 2, 0)) return 1; // THIS WILL FAIL ON PURPOSE!
                                                                               // I MADE THE MAX BLOCKS VALUE*
                                                                               // SET TO 2! THE SERVER SENDS
                                                                               // 3 DATA BLOCKS BY DEFAULT!
                                                                               //
                                                                               // EITHER CHANGE THE VALUE OR
                                                                               // COMMENT OUT A DATA BLOCK IN
                                                                               // THE SERVER CODE


    // CHECK BLOCK COUNT

    printf("Block Count: %u\n", block_count);


    // PRINT BLOCKS NICELY

    for (int i = 0; i < block_count; i++) {
        printf("--\nType: %u\nSign: %u\nLen: %u\nData: ", block_buff[i].type, block_buff[i].sign, 
                block_buff[i].len);

        uint8_t type = block_buff[i].type;

        for (int j = 0; j < block_buff[i].len; j++) {
            if (type == mn_s8) {
                //printf("%c, ", ((char *)block_buff[i].dat)[j]);
                printf("%s", ((char *)block_buff[i].dat));
                break;
            } else if (type == mn_s16) {
                printf("%u, ", ((uint16_t *)block_buff[i].dat)[j]);
            } else if (type == mn_s32) {
                printf("%u", ((uint32_t *)block_buff[i].dat)[j]);
            }
        }

        printf("\n");
    }


    // FREE DATA BLOCK BUFFER

    free(block_buff);


    // CLOSE CLIENT

    mn_client_close(&client);

    return 0;
    
}
