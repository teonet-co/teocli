/** 
 * \file   main.c
 * \author Kirill Scherba <kirill@scherba.ru>
 * 
 * \example main.c
 * 
 * This is basic example of Teocli library. This application connect to network 
 * L0 server, initialize (login) at the L0 server, and send and receive data to from network peer.  
 *
 * See server example parameters at:  
 *   https://gitlab.ksproject.org/teonet/teocli/blob/master/README.md#basic-teocli-example
 * 
 * ### This application parameters:  
 * 
 * **Usage:**   ./teocli <client_name> <server_address> <server_port> <peer_name> [message]  
 * 
 * **Example:** ./teocli C3 127.0.0.1 9000 teostream "Story about this world!"  
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "libteol0/teonet_l0_client.h"

#define TL0CN_VERSION "0.0.1"  

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
    printf("Teol0cli_n example ver " TL0CN_VERSION " (native client)\n\n");
    
    // Check parameters
    if(argc < 5) {
        
        printf("Usage: "
               "%s <client_name> <server_address> <server_port> <peer_name> "
               "[message]\n", argv[0]);
        
        exit(EXIT_SUCCESS);
    }
    
    // Teonet L0 server parameters
    const char *host_name = argv[1]; //"C3";
    const char *TCP_IP = argv[2]; //"127.0.0.1"; //"10.12.35.53"; // 
    const int TCP_PORT = atoi(argv[3]); //9000;
    const char *peer_name = argv[4]; //"teostream";
    const char *msg;
    if(argc > 5) msg = argv[5];
    else msg = "Hello";
    
    // Send packet buffer
    size_t snd;
    #define BUFFER_SIZE L0_BUFFER_SIZE
    char packet[BUFFER_SIZE];
    teoLNullCPacket *pkg = (teoLNullCPacket*) packet;
    
    // Receive packet buffer
    ssize_t rc;
    //char buf[BUFFER_SIZE];
    //teoLNullCPacket *cp = (teoLNullCPacket*)buf;
    
    // Initialize L0 Client library
    teoLNullInit();

    // Connect to L0 server
    teoLNullConnectData *con = teoLNullClientConnect(TCP_PORT, TCP_IP);    
    if(con->fd > 0) {
        
        // Send (1) Initialize L0 connection
        size_t pkg_length = teoLNullClientLogin(packet, BUFFER_SIZE, host_name);
        if((snd = teoLNullPacketSend(con->fd, pkg, pkg_length)) >= 0);
        if(snd == -1) perror(strerror(errno));
        printf("\nSend %d bytes packet of %d bytes buffer initialize packet to L0 server\n", 
                (int)snd, (int)pkg_length);
        
        // Send (2) peer list request to peer
        pkg_length = teoLNullPacketCreate(packet, BUFFER_SIZE, CMD_L_PEERS, 
                peer_name, NULL, 0);
        if((snd = teoLNullPacketSend(con->fd, pkg, pkg_length)) >= 0);
        printf("Send %d bytes packet of %d bytes buffer packet to L0 server to peer %s, "
               "cmd = %d\n", 
               (int)snd, (int)pkg_length, peer_name, CMD_L_PEERS);
        
        // Send (3) command echo
        pkg_length = teoLNullPacketCreate(packet, BUFFER_SIZE, CMD_L_ECHO, 
                peer_name, msg, strlen(msg) + 1);
        if((snd = teoLNullPacketSend(con->fd, pkg, pkg_length)) >= 0);
        if(snd == -1) perror(strerror(errno));
        printf("Send %d bytes packet of %d bytes buffer to L0 server to peer %s, "
               "data: %s\n", 
               (int)snd, (int)pkg_length, peer_name, msg);

        // Receive (1) answer from server      
        while((rc = teoLNullPacketRecvS(con)) == -1);  
        
        // Process received data
        if(rc > 0) {
            
            teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
            char *data = cp->peer_name + cp->peer_name_length;
            printf("Receive %d bytes: %d bytes data from L0 server, "
                    "from peer %s, data: %s\n\n", 
                    (int)rc, cp->data_length, cp->peer_name, data);
        }
        
        // Receive (2) answer from server
        while((rc = teoLNullPacketRecvS(con)) == -1);
        
        // Process received data
        if(rc > 0) {

            teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
            char *data = cp->peer_name + cp->peer_name_length;
            printf("Receive %d bytes: %d bytes data from L0 server, "
                    "from peer %s, data: %s\n\n", 
                    (int)rc, cp->data_length, cp->peer_name, data);
        }
        
        // Close connection
        teoLNullClientDisconnect(con);
    }
    
    // Cleanup L0 Client library
    teoLNullCleanup();
    
    return (EXIT_SUCCESS);
}
