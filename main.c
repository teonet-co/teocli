/* 
 * File:   main.c
 * Author: Kirill Scherba <kirill@scherba.ru>
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
    
    printf("Teol0cli_n example ver " TL0CN_VERSION " (native client)\n\n");
    
    // Initialize L0 Client library
    teoLNullInitClient();

    // Teonet L0 server parameters
    const char *peer_name = "teostream";
    const char *host_name = "C3";
    const char *TCP_IP = "10.12.35.53"; // "127.0.0.1";
    const int TCP_PORT = 9000;
    #define CMD_ECHO 65
    
    // Packet buffer
    #define BUFFER_SIZE 2048
    char packet[BUFFER_SIZE];
    teoLNullCPacket *pkg = (teoLNullCPacket*) packet;
    
    // Connect to L0 server
    int fd = teoLNullClientCreate(TCP_PORT, TCP_IP);
    if(fd > 0) {
        
        // Initialize L0 connection
        size_t snd;
        size_t pkg_length = teoLNullInit(packet, BUFFER_SIZE, host_name);
        if((snd = teoLNullPacketSend(fd, pkg, pkg_length)) >= 0);
        if(snd == -1) perror(strerror(errno));
        printf("\nSend %d bytes of %d buffer initialize packet to L0 server\n", 
                (int)snd, (int)pkg_length);

        // Send command message
        const char *msg = "Hello";
        pkg_length = teoLNullPacketCreate(packet, BUFFER_SIZE, CMD_ECHO, 
                peer_name, msg, strlen(msg) + 1);
        if((snd = teoLNullPacketSend(fd, pkg, pkg_length)) >= 0);
        if(snd == -1) perror(strerror(errno));
        printf("Send %d bytes packet of %d buffer to L0 server to peer %s, data: %s\n", 
               (int)snd, (int)pkg_length, peer_name, msg);

        // Receive answer from server
        char buf[BUFFER_SIZE];
        size_t rc;
        while((rc = read(fd, buf, BUFFER_SIZE)) == -1);

        // Process received data
        if(rc > 0) {

            teoLNullCPacket *cp = (teoLNullCPacket*)buf;
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
