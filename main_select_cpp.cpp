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

#include "libteol0/teocli"

/**
 * Application parameters structure
 */
struct AppParameters {

    const char *client_name;
    const char *tcp_server;
    int tcp_port;
    const char *peer_name;
    const char *message;

};

/**
 * Teonet L0 client event callback
 *
 * @param cli
 * @param event
 * @param data
 * @param data_len
 * @param user_data
 */
static void event_cb(teo::Teocli &cli, teo::Events event, void *data,
            size_t data_len, void *user_data) {
    
    const AppParameters *param = (const AppParameters *)user_data;

    switch(event) {

        case EV_L_CONNECTED: {
        
            if(cli.connected() > 0) {

                std::cout << "Successfully connect to server\n";
                
                // Send (1) peer list request to peer, command CMD_L_PEERS
                ssize_t snd = cli.send(CMD_L_PEERS, param->peer_name, NULL, 0);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", "
                       "cmd = " << CMD_L_PEERS << " (CMD_L_PEERS)\n";
                
                // Send (2) clients list request to peer, command CMD_L_L0_CLIENTS
                snd = cli.send(CMD_L_L0_CLIENTS, param->peer_name, NULL, 0);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", "
                       "cmd = " << CMD_L_L0_CLIENTS << " (CMD_L_L0_CLIENTS)\n";

                // Send (3) echo request to peer, command CMD_L_ECHO
                snd = cli.sendEcho(param->peer_name, param->message);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", " 
                       "cmd = " << CMD_L_ECHO << " (CMD_L_ECHO), "
                       "data: " << param->message << "\n";

                // Show empty line
                std::cout << "\n";
            }
            else std::cout << "Can't connect to server\n";

        } break;

        case EV_L_DISCONNECTED: std::cout << "Disconnected ...\n"; break;

        case EV_L_RECEIVED: {
        
            // Receive answer from server
            const size_t rc = data_len;
            teo::Packet *cp = cli.packet();

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

//teo::Teocli *cli;

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
    std::cout << "Teonet c++ L0 client with Select and Event Callback example "
                 "version " TL0CN_VERSION " (Native TCP Client)\n\n";

    // Check application parameters
    if(argc < 5) {

        std::cout << "Usage: " << argv[0] << 
               " <client_name> <server_address> <server_port> <peer_name> "
               "[message]\n";

        exit(EXIT_SUCCESS);
    }

    // Teonet L0 server parameters
    AppParameters parameters = { 
        argv[1], // This client name 
        argv[2], // L0 tcp_server
        atoi(argv[3]), // L0 tcp_port
        argv[4], // Peer name to send test messages to
        argc > 5 ? argv[5] : "Hello" // Test message
    };

    // Create Teocli object, Initialize L0 Client library and connect to L0 server
    teo::Teocli *cli = new teo::Teocli(parameters.client_name, 
        parameters.tcp_server, parameters.tcp_port, event_cb, &parameters);

    if(cli->connected() > 0) {

        unsigned long num = 0;
        const int timeout = 50;

	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(cli->eventLoopE, cli, 60, 0);
	#else
        // Event loop
        while(cli->eventLoop(timeout)) {

            // Send Echo command every second
            if( !(num % (1000 / timeout)) )
                cli->sendEcho(parameters.peer_name, parameters.message);

            num++;
        }
	#endif

    }
    
    #ifdef __EMSCRIPTEN__
    #else
    delete(cli);
    #endif

    return (EXIT_SUCCESS);
}