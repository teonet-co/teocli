/**
 * \file   main_select.cpp
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
 * **Usage:**   ./teocli_s_cpp <client_name> <server_address> <server_port> <peer_name> [message]
 *
 * **Example:** ./teocli_s_cpp C3 127.0.0.1 9000 teostream "Story about this world!"
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
 * Created on May 30, 2017, 13:27
 */

#include <iostream>
#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "libteol0/teocli.hpp"

// Application constants
#define SEND_MESSAGE_AFTER_MIN  15000 /* 16667 */ // uSec (mSec * 1000)
#define SEND_MESSAGE_AFTER  1000000
#define RECONNECT_AFTER 3000000 // uSec (mSec * 1000)
#define SHOW_STATISTIC_AFTER 500000 // uSec (mSec * 1000)

/**
 * Teonet L0 client event callback
 *
 * @param cli
 * @param event
 * @param data
 * @param data_len
 * @param user_data
 */
/*static void event_cb(void *con, teo::Events event, void *data,
            size_t data_len, void *user_data) {

    const struct app_parameters *param = (const struct app_parameters *)user_data;
    teo::Teocli *cli = (teo::Teocli *)con;

    switch(event) {

        case EV_L_CONNECTED: {

            if(cli->connected() > 0) {

                std::cout << "Successfully connect to server\n";

                // Send (1) peer list request to peer, command CMD_L_PEERS
                ssize_t snd = cli->send(CMD_L_PEERS, param->peer_name, NULL, 0);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", "
                       "cmd = " << CMD_L_PEERS << " (CMD_L_PEERS)\n";

                // Send (2) clients list request to peer, command CMD_L_L0_CLIENTS
                snd = cli->send(CMD_L_L0_CLIENTS, param->peer_name, NULL, 0);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", "
                       "cmd = " << CMD_L_L0_CLIENTS << " (CMD_L_L0_CLIENTS)\n";

                // Send (3) echo request to peer, command CMD_L_ECHO
                snd = cli->sendEcho(param->peer_name, param->msg);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", "
                       "cmd = " << CMD_L_ECHO << " (CMD_L_ECHO), "
                       "data: " << param->msg << "\n";

                // Show empty line
                std::cout << "\n";
            }
            else std::cout << "Can't connect to server\n";

        } break;

        case EV_L_DISCONNECTED: std::cout << "Disconnected ...\n"; break;

        case EV_L_RECEIVED: {

            // Receive answer from server
            const size_t rc = data_len;
            teo::Packet *cp = cli->packet();

            std::cout << "Receive " << rc << " bytes: " << cp->data_length <<
                " bytes data from L0 server, "
                "from peer '" << cp->peer_name << "', cmd = " << int(cp->cmd) << "\n";

            // Process commands
            switch(cp->cmd) {

                case CMD_L_PEERS_ANSWER: {

                    // Show peer list
                    if(cp->data_length > 1) {

                        ksnet_arp_data_ar *arp_data_ar = (ksnet_arp_data_ar *)
                                (cp->peer_name + cp->peer_name_length);
                        const char *ln =
                                "--------------------------------------------"
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

                } break;

                case CMD_L_L0_CLIENTS_ANSWER: {

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

                } break;

                case CMD_L_ECHO_ANSWER: {

                    std::cout << "Got echo answer command\n";
                    data = cp->peer_name + cp->peer_name_length;
                    int trip_time = teoLNullProccessEchoAnswer((char *)data);

                    // Show data
                    std::cout << "Data: " << (char*)data << "\n";

                    // Show trip time
                    std::cout << "Trip time: " << trip_time << " ms\n\n";

                } break;

                case CMD_L_AUTH_LOGIN_ANSWER: {

                    std::cout << "Got answer from authentication server\n";

                    const char *auth_data = (const char *)
                            (cp->peer_name + cp->peer_name_length);

                    // Show data
                    std::cout << "Data: " << auth_data << "\n\n";
                }
                break;

                case CMD_L_ECHO: {

                    std::cout << "Got echo command\n";

                } break;

                default: {

                    std::cout << "Got unknown command\n";

                } break;
            }

        } break;

        default:
            break;
    }
}
*/
void event_cb(void *con, teoLNullEvents event, void *data,
            size_t data_len, void *user_data) {


    const struct app_parameters *param = (const struct app_parameters *)user_data;

    switch(event) {

        case EV_L_CONNECTED:
        {
            int *fd = (int *)data;
            if(*fd > 0) {

                printf("Successfully connect to server\n");

                // Send (1) Initialization packet to L0 server
/*                ssize_t snd = teoLNullLogin(con, param->host_name, param->proto);
                if(snd == -1) {
                    perror(strerror(errno));
                } else {
                    printf("\nSend %d bytes packet to L0 server Initialization packet\n", (int)snd);
                }
*/
                // Send (2) peer list request to peer, command CMD_L_PEERS
                ssize_t snd = l0_send_msg(con, CMD_L_PEERS, param->peer_name, NULL, 0, param->proto);
                if(snd == -1) {
                    perror(strerror(errno));
                } else {
                    printf("Send %d bytes packet to L0 server to peer %s, cmd = %d (CMD_L_PEERS)\n",
                       (int)snd, param->peer_name, CMD_L_PEERS);
                }

                // Send (3) clients list request
                snd = l0_send_msg(con, CMD_L_L0_CLIENTS, param->peer_name, NULL, 0, param->proto);
                if(snd == -1) {
                    perror(strerror(errno));
                } else {
                    printf("Send %d bytes packet to L0 server to peer %s, "
                       "cmd = %d (CMD_L_L0_CLIENTS)\n",
                       (int)snd, param->peer_name, CMD_L_L0_CLIENTS);
                }

                // Send (3) echo request to peer, command CMD_L_ECHO
                //
                // Add current time to the end of message (it should be return
                // back by server)
                snd = l0_send_echo(con, param->peer_name, param->msg, param->proto);
                if(snd == -1) {
                    perror(strerror(errno));
                } else {
                    printf("Send %d bytes packet to L0 server to peer %s, "
                       "cmd = %d (CMD_L_ECHO), "
                       "data: %s\n",
                       (int)snd, param->peer_name, CMD_L_ECHO, param->msg);
                }

                printf("\n");
            } else {
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

            printf("Receive %d bytes: %hu bytes data from L0 server, "
                    "from peer %s, cmd = %hhu\n",
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
                } break;

                case CMD_L_L0_CLIENTS_ANSWER: {

                    // Show peer list
                    teonet_client_data_ar *client_data_ar = (teonet_client_data_ar *)
                            (cp->peer_name + cp->peer_name_length);
                    const char *ln = "--------------------------------------------"
                                     "---------\n";
                    printf("%sClients (%u): \n%s", ln, client_data_ar->length, ln);
                    int i = 0;
                    for(i = 0; i < (int)client_data_ar->length; ++i) {
                        printf("%-12s\n", client_data_ar->client_data[i].name);
                    }
                    printf("%s", ln);
                } break;

                case CMD_L_ECHO_ANSWER:
                {
                    printf("Got echo answer command\n");
                    data = cp->peer_name + cp->peer_name_length;
                    int trip_time = teoLNullProccessEchoAnswer((const char*)data);

                    printf("Data: %s\n", (char*)data);
                    printf("Trip time: %d ms\n\n", trip_time);
                } break;

                case CMD_L_ECHO:
                {
                    printf("Got echo command\n");
                } break;

                case CMD_L_AUTH_LOGIN_ANSWER:
                {
                    printf("Got answer from authentication server\n");
                    const char *auth_data = (const char *) (cp->peer_name + cp->peer_name_length);
                    printf("Data: %s\n\n", auth_data);
                    if (param->proto == TR_UDP) {
                        ((trudpChannelData *)con)->connected_f = 1;
                        send_l0_event_udp(con, EV_L_CONNECTED, &((trudpChannelData *)con)->fd,
                                sizeof(((trudpChannelData *)con)->fd), NULL);
                    }
                } break;

                default: {
                    printf("Got unknown command\n");
                } break;
            }

        } break;

        default:
            break;
    }
}
#include  <signal.h>
volatile sig_atomic_t quit_flag = 0;

void INThandler(int sig)
{
    printf("Catch CTRL-C SIGNAL !!!\n");
    quit_flag = 1;
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
    signal(SIGINT, INThandler); 

    // Welcome message
    std::cout << "Teonet c++ L0 client with Select and Event Callback example "
                 "version " TL0CN_VERSION " (Native TCP/UDP Client)\n\n";

    // Check application parameters
    if(argc < 5) {

        std::cout << "Usage: " << argv[0] <<
               " <client_name> <server_address> <server_port> <peer_name> "
               "[message]\n";

        exit(EXIT_SUCCESS);
    }
    // Teonet L0 server parameters
    struct app_parameters parameters;
    parameters.host_name = argv[1]; //"C3";
    parameters.tcp_server = argv[2]; //"127.0.0.1"; //"10.12.35.53"; //
    parameters.tcp_port = atoi(argv[3]); //9000;
    parameters.peer_name = argv[4]; //"teostream";
    parameters.proto = TR_UDP;
    if(argc > 5) parameters.msg = argv[5];
    else parameters.msg = "Hello";

    auto cli = teo::Teocli::create_connection(event_cb, &parameters);
    
    // Create Teocli object, Initialize L0 Client library and connect to L0 server
/*    teo::Teocli *cli = new teo::Teocli(parameters.host_name,
        parameters.tcp_server, parameters.tcp_port, event_cb, &parameters);
        
*/        

    uint32_t tt = 0, tt_s = 0, tt_c = 0, tt_ss = 0;
    const int DELAY = 500000; // uSec
    unsigned long num = 0;
 

        while(!quit_flag) {
          cli->eventLoop(SEND_MESSAGE_AFTER); 
          // Current timestamp
          tt = trudpGetTimestamp();

          // Connect
          if(!cli->connected() && (tt - tt_c) > RECONNECT_AFTER) {

              cli->channel_clean(&parameters);
              tt_c = tt;
          }

          // When connected
          if(cli->connected() > 0) {
              // Send Echo command every 1 second
              if((tt - tt_s) > SEND_MESSAGE_AFTER * 1) {
                  cli->sendEcho(parameters.peer_name, parameters.msg);
                  tt_s = tt;
              } else {
                cli->keep_con();
              }
          }
        }
        cli->disconnect();

   return (EXIT_SUCCESS);
}
