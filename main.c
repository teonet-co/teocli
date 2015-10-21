/* 
 * File:   main.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 * 
 * See server example parameters at: 
 * https://gitlab.ksproject.org/teonet/teonet/wikis/l0-server
 * 
 * This application parameters:
 * Usage:   ./teocli <client_name> <server_address> <server_port> <peer_name>
 * example: ./teocli C3 127.0.0.1 9000 teostream "Story about this world!"
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
    
    // Initialize L0 Client library
    teoLNullInitClient();

    // Teonet L0 server parameters
    const char *host_name = argv[1]; //"C3";
    const char *TCP_IP = argv[2]; //"127.0.0.1"; //"10.12.35.53"; // 
    const int TCP_PORT = atoi(argv[3]); //9000;
    const char *peer_name = argv[4]; //"teostream";
    const char *msg;
    if(argc > 5) msg = argv[5];
    else msg = "Hello";
    
    // System commands
    #define CMD_ECHO 65
    #define CMD_PEERS 72
    
    // Send packet buffer
    size_t snd;
    #define BUFFER_SIZE 2048
    char packet[BUFFER_SIZE];
    teoLNullCPacket *pkg = (teoLNullCPacket*) packet;
    
    // Receive packet buffer
    size_t rc;
    char buf[BUFFER_SIZE];
    teoLNullCPacket *cp = (teoLNullCPacket*)buf;
    
    // Connect to L0 server
    int fd = teoLNullClientCreate(TCP_PORT, TCP_IP);
    if(fd > 0) {
        
        // Initialize L0 connection
        size_t pkg_length = teoLNullInit(packet, BUFFER_SIZE, host_name);
        if((snd = teoLNullPacketSend(fd, pkg, pkg_length)) >= 0);
        if(snd == -1) perror(strerror(errno));
        printf("\nSend %d bytes of %d buffer initialize packet to L0 server\n", 
                (int)snd, (int)pkg_length);
        
        // Send peer list request to peer
        pkg_length = teoLNullPacketCreate(packet, BUFFER_SIZE, 
                CMD_PEERS, peer_name, NULL, 0);
        if((snd = teoLNullPacketSend(fd, pkg, pkg_length)) >= 0);
        printf("Send %d bytes of %d buffer packet to L0 server to peer %s, cmd = %d\n", 
                (int)snd, (int)pkg_length, peer_name, CMD_PEERS);
        
        // Receive answer from server
        while((rc = read(fd, buf, BUFFER_SIZE)) == -1);
        
        // Process received data
        if(rc > 0) {

            char *data = cp->peer_name + cp->peer_name_length;
            printf("Receive %d bytes: %d bytes data from L0 server, "
                    "from peer %s, data: %s\n\n", 
                    (int)rc, cp->data_length, cp->peer_name, data);
        }

        // Send command echo
        pkg_length = teoLNullPacketCreate(packet, BUFFER_SIZE, CMD_ECHO, 
                peer_name, msg, strlen(msg) + 1);
        if((snd = teoLNullPacketSend(fd, pkg, pkg_length)) >= 0);
        if(snd == -1) perror(strerror(errno));
        printf("Send %d bytes packet of %d buffer to L0 server to peer %s, data: %s\n", 
               (int)snd, (int)pkg_length, peer_name, msg);

        // Receive answer from server        
        while((rc = read(fd, buf, BUFFER_SIZE)) == -1);

        // Process received data
        if(rc > 0) {

            char *data = cp->peer_name + cp->peer_name_length;
            printf("Receive %d bytes: %d bytes data from L0 server, "
                    "from peer %s, data: %s\n\n", 
                    (int)rc, cp->data_length, cp->peer_name, data);
        }

        // Close connection
        close(fd);
    }
    
    // Cleanup L0 Client library
    teoLNullCleanupClient();
    
    return (EXIT_SUCCESS);
}
