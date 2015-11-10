/** 
 * \file   main_select.c
 * \author Kirill Scherba <kirill@scherba.ru>
 * 
 * \example main_select.c
 * 
 * This is example of Teocli library which use select to check that data is 
 * available ready to read. This application connect to network 
 * L0 server, initialize (login) at the L0 server, and send and receive data to 
 * from network peer.  
 *
 * See server example parameters at:  
 *   https://gitlab.ksproject.org/teonet/teocli/blob/master/README.md#basic-teocli-example
 * 
 * ### This application parameters:  
 * 
 * **Usage:**   ./teocli_s <client_name> <server_address> <server_port> <peer_name> [message]  
 * 
 * **Example:** ./teocli_s C3 127.0.0.1 9000 teostream "Story about this world!"  
 * 
 * ### This application algorithm:
 *  
 * *  Connect to L0 server with server_address and server_port parameters
 * *  Send ClientLogin request with client_name parameter
 * *  Send CMD_L_PEERS request to peer_name server
 * *  Receive peers data from peer_name server
 * *  Send CMD_L_ECHO request to peer_name server
 * *  Receive peers data from peer_name server
 * *  Close connection
 *
 * Created on October 19, 2015, 3:51 PM
 */

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !(defined(_WIN32) || defined(_WIN64))
#include <unistd.h>
#endif
#include <errno.h>
#include <sys/timeb.h> 

#include "libteol0/teonet_l0_client.h"

#define TL0CNS_VERSION "0.0.1"  

/**
 * Application parameters structure
 */
struct app_parameters {
    
    const char *host_name;
    const char *tcp_server;
    int tcp_port;
    const char *peer_name;
    const char *msg;
    
};

/**
 * Teonet L0 client event callback
 * 
 * @param con
 * @param event
 * @param data
 * @param data_len
 * @param user_data
 */
void event_cb(void *con, teoLNullEvents event, void *data, 
            size_t data_len, void *user_data) {
    
    const struct app_parameters *param = user_data;
       
    switch(event) {
                
        case EV_L_CONNECTED:
        {
            int *fd = data;
            if(*fd > 0) {
              
                printf("Successfully connect to server\n");
                
                // Send (1) Initialization packet to L0 server
                ssize_t snd = teoLNullLogin(con, param->host_name);
                if(snd == -1) perror(strerror(errno));
                printf("\nSend %d bytes packet to L0 server, "
                       "Initialization packet\n", 
                       (int)snd);

                // Send (2) peer list request to peer, command CMD_L_PEERS
                snd = teoLNullSend(con, CMD_L_PEERS, param->peer_name, NULL, 0);        
                printf("Send %d bytes packet to L0 server to peer %s, "
                       "cmd = %d (CMD_L_PEERS)\n", 
                       (int)snd, param->peer_name, CMD_L_PEERS);

                // Send (3) echo request to peer, command CMD_L_ECHO
                //
                // Add current time to the end of message (it should be return 
                // back by server)
                snd = teoLNullSendEcho(con, param->peer_name, param->msg);
                if(snd == -1) perror(strerror(errno));
                printf("Send %d bytes packet to L0 server to peer %s, "
                       "cmd = %d (CMD_L_ECHO), " 
                       "data: %s\n", 
                       (int)snd, param->peer_name, CMD_L_ECHO, param->msg);
                
                // Show empty line
                printf("\n");         
                
            } 
            else {
                
                printf("Can't connect to server\n");
            }
            
        } break;
            
        case EV_L_DISCONNECTED:
            printf("Disconnected ...\n");
            break;
        
        case EV_L_RECEIVED:
        {
            // Receive answer from server 
            const size_t rc = data_len;
            teoLNullCPacket *cp = (teoLNullCPacket*) data;
            
            printf("Receive %d bytes: %d bytes data from L0 server, "
                    "from peer %s, cmd = %d\n", 
                    (int)rc, cp->data_length, cp->peer_name, cp->cmd);

            // Process commands
            switch(cp->cmd) {

                case CMD_L_PEERS_ANSWER:
                {
                    // Show peer list
                    if(cp->data_length > 1) {
                        
                        ksnet_arp_data_ar *arp_data_ar = (ksnet_arp_data_ar *)
                                (cp->peer_name + cp->peer_name_length);
                        const char *ln = 
                                "--------------------------------------------"
                                "---------\n";
                        printf("%sPeers (%d): \n%s", ln, arp_data_ar->length, ln);
                        int i;
                        for(i = 0; i < (int)arp_data_ar->length; i++) {

                            printf("%-12s(%2d)   %-15s   %d %8.3f ms\n", 
                                arp_data_ar->arp_data[i].name, 
                                arp_data_ar->arp_data[i].data.mode,
                                arp_data_ar->arp_data[i].data.addr,
                                arp_data_ar->arp_data[i].data.port,
                                arp_data_ar->arp_data[i].data.last_triptime);

                        }
                        printf("%s", ln);
                    }
                } break;

                case CMD_L_ECHO_ANSWER:
                {
                    data = cp->peer_name + cp->peer_name_length;
                    int trip_time = teoLNullProccessEchoAnswer(data);

                    // Show data
                    printf("Data: %s\n", (char*)data);

                    // Show trip time
                    printf("Trip time: %d ms\n", trip_time);

                } break;
                
                case CMD_L_ECHO:
                {
                    printf("Got echo command\n");
                    data = cp->peer_name + cp->peer_name_length;
                    teoLNullSend(con, CMD_L_ECHO_ANSWER, cp->peer_name, data, 
                        cp->data_length);
                    
                } break;   
                
                default:
                    printf("Got unknown command\n");
                    break;
            }
            
        } break;
            
        default:
            break;
    }
}

/**
 * Main L0 Native client example function
 *
 * @param argc Number of arguments
 * @param argv Arguments array
 * 
 * @return
 */
int main(int argc, char** argv) {
    
    // Welcome message
    printf("Teonet L0 client with select example ver " TL0CNS_VERSION 
           " (native client)\n\n");
    
    // Check application parameters
    if(argc < 5) {
        
        printf("Usage: "
               "%s <client_name> <server_address> <server_port> <peer_name> "
               "[message]\n", argv[0]);
        
        exit(EXIT_SUCCESS);
    }
    
    // Teonet L0 server parameters
    struct app_parameters param;
    param.host_name = argv[1]; //"C3";
    param.tcp_server = argv[2]; //"127.0.0.1"; //"10.12.35.53"; // 
    param.tcp_port = atoi(argv[3]); //9000;
    param.peer_name = argv[4]; //"teostream";
    if(argc > 5) param.msg = argv[5];
    else param.msg = "Hello";
    
    // Initialize L0 Client library
    teoLNullInit();

    // Connect to L0 server
    teoLNullConnectData *con = teoLNullConnectE(param.tcp_server, param.tcp_port, 
        event_cb, &param);    
    
    if(con->fd > 0) {
        
        unsigned long num = 0;
        const int timeout = 50;
        
        // Event loop
        while(teoLNullReadEventLoop(con, timeout)) {

            // Send Echo command every second
            if( !(num % (1000 / timeout)) )
                teoLNullSendEcho(con, param.peer_name, param.msg);
            
            num++;
        }
            
        // Close connection
        teoLNullDisconnect(con);
    }
    
    // Cleanup L0 Client library
    teoLNullCleanup();
    
    return (EXIT_SUCCESS);
}