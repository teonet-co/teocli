/**
 * \file   main.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * \example main.c
 *
 * This is basic example of Teocli library. This application connect to network
 * L0 server, initialize (login) at the L0 server, and send and receive data to
 * from network peer.
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

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <errno.h>

#include "libteol0/teonet_l0_client.h"


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
    printf("Teonet L0 client example version " TL0CN_VERSION " (Native TCP Client)\n\n");

    // Check application parameters
    if(argc < 5) {

        printf("Usage: "
               "%s <client_name> <server_address> <server_port> <peer_name> "
               "[message]\n", argv[0]);

        exit(EXIT_SUCCESS);
    }

    // Teonet L0 server parameters
    const char *host_name = argv[1]; //"C3";
    const char *TCP_SERVER = argv[2]; //"127.0.0.1"; //"10.12.35.53"; //
    const int TCP_PORT = atoi(argv[3]); //9000;
    const char *peer_name = argv[4]; //"teostream";
    const char *msg;
    if(argc > 5) msg = argv[5];
    else msg = "Hello";

    // Define receive packet size and data pointer variables
    ssize_t rc;
    char *data = NULL;

    // Initialize L0 Client library
    teoLNullInit();

    // Connect to L0 server
    teoLNullConnectData *con = teoLNullConnect(TCP_SERVER, TCP_PORT);
    if(con->status > 0) {

        // Send (1.1) Initialization packet to L0 server
        ssize_t snd = teoLNullLogin(con, host_name);
        if(snd == -1) perror(strerror(errno));
        printf("\nSend %d bytes packet to L0 server, Initialization packet\n",
                (int)snd);

        // Get L0 Server answer (1.2)
        if((rc = teoLNullRecvTimeout(con, 1000000)) == -1) {

            printf("Can't get answer from L0 server during timeout\n");

        }
        // Show L0 answer and continue send and receive data
        else {

            // Show L0 answer
            teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
            data = cp->peer_name + cp->peer_name_length;
            printf("Receive %d bytes: %hu bytes data from L0 server, "
                    "from peer %s, cmd = %hhu, data: %s\n\n",
                    (int)rc, cp->data_length, cp->peer_name, cp->cmd,
                    cp->data_length ? data : "");

            // Send (2) peer list request to peer, command CMD_L_PEERS
            snd = teoLNullSend(con, CMD_L_PEERS, peer_name, NULL, 0);
            printf("Send %d bytes packet to L0 server to peer %s, "
                   "cmd = %d (CMD_L_PEERS)\n",
                   (int)snd,peer_name, CMD_L_PEERS);

            // Send (2.5) clients list request to peer, command CMD_L_L0_CLIENTS
            snd = teoLNullSend(con, CMD_L_L0_CLIENTS, peer_name, NULL, 0);
            printf("Send %d bytes packet to L0 server to peer %s, "
                   "cmd = %d (CMD_L_L0_CLIENTS)\n",
                   (int)snd, peer_name, CMD_L_L0_CLIENTS);

            // Send (3) echo request to peer, command CMD_L_ECHO
            //
            snd = teoLNullSendEcho(con, peer_name, msg);
            if(snd == -1) perror(strerror(errno));
            printf("Send %d bytes packet to L0 server to peer %s, "
                   "cmd = %d (CMD_L_ECHO), "
                   "data: %s\n",
                   (int)snd, peer_name, CMD_L_ECHO, msg);

            // Show empty line
            printf("\n");

            // Receive (1) answer from server, CMD_L_PEERS_ANSWER
            while((rc = teoLNullRecv(con)) == -1) teoLNullSleep(50);

            // Process received data
            if(rc > 0) {

                teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
                printf("Receive %d bytes: %hu bytes data from L0 server, "
                        "from peer %s, cmd = %hhu\n",
                        (int)rc, cp->data_length, cp->peer_name, cp->cmd);

                // Process CMD_L_PEERS_ANSWER
                if(cp->cmd == CMD_L_PEERS_ANSWER && cp->data_length > 1) {

                    // Show peer list
                    ksnet_arp_data_ar *arp_data_ar = (ksnet_arp_data_ar *)
                            (cp->peer_name + cp->peer_name_length);
                    const char *ln = "--------------------------------------------"
                                     "---------\n";
                    printf("%sPeers (%u): \n%s", ln, arp_data_ar->length, ln);
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
            }

            // Show empty line
            printf("\n");

            // Receive (1.5) answer from server, CMD_L_L0_CLIENTS_ANSWER
            while((rc = teoLNullRecv(con)) == -1) teoLNullSleep(50);

            // Process received data
            if(rc > 0) {

                teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
                printf("Receive %d bytes: %hu bytes data from L0 server, "
                        "from peer %s, cmd = %hhu\n",
                        (int)rc, cp->data_length, cp->peer_name, cp->cmd);

                // Process CMD_L_L0_CLIENTS_ANSWER
                if(cp->cmd == CMD_L_L0_CLIENTS_ANSWER && cp->data_length > 1) {

                    // Show peer list
                    teonet_client_data_ar *client_data_ar = (teonet_client_data_ar *)
                            (cp->peer_name + cp->peer_name_length);
                    const char *ln = "--------------------------------------------"
                                     "---------\n";
                    printf("%sClients (%u): \n%s", ln, client_data_ar->length, ln);
                    int i;
                    for(i = 0; i < (int)client_data_ar->length; i++) {

                        printf("%-12s\n", client_data_ar->client_data[i].name);
                    }
                    printf("%s", ln);
                }
            }

            // Show empty line
            printf("\n");

            // Receive (2) answer from server
            while((rc = teoLNullRecv(con)) == -1) teoLNullSleep(50);

            // Process received data
            if(rc > 0) {

                teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
                data = cp->peer_name + cp->peer_name_length;
                printf("Receive %d bytes: %hu bytes data from L0 server, "
                        "from peer %s, cmd = %hhu, data: %s\n",
                        (int)rc, cp->data_length, cp->peer_name, cp->cmd, data);

                // Process CMD_L_ECHO_ANSWER
                if(cp->cmd == CMD_L_ECHO_ANSWER) {

                    // Get time from answers data
                    int trip_time = teoLNullProccessEchoAnswer(data);

                    // Show trip time
                    printf("Trip time: %d ms\n", trip_time);
                }
            }

            // Show empty line
            printf("\n");

            // Check received ECHO
            printf("Test result: %s\n", ((data != NULL && !strcmp(msg, data)) ? "OK" : "ERROR"));
            printf("Press Ctrl+C to exit\n\n");

            // Receive other data
            for(;;) {

                while((rc = teoLNullRecv(con)) == -1) teoLNullSleep(50);
                if(rc > 0) {

                    teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
                    data = cp->peer_name + cp->peer_name_length;
                    printf("Receive %d bytes: %hu bytes data from L0 server, "
                            "from peer %s, cmd = %hhu, data: %s\n",
                            (int)rc, cp->data_length, cp->peer_name, cp->cmd,
                            cp->data_length ? data : "");

                    // Echo answer command sent automatically by client
                }
                else if(rc == 0) {

                    printf("Disconnected ...\n\n");
                    break;
                }
            }
        }

        // Close connection
        teoLNullDisconnect(con);
    }

    // Cleanup L0 Client library
    teoLNullCleanup();

    return (EXIT_SUCCESS);
}
