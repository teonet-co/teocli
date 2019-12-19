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

#include <memory>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <iostream>
#include <functional>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "teocli.hpp"

class Teo;

using ConnectCb =  std::function<void (int type, Teo *cli)>;
using DisconnectCb = std::function<void ()>;
using IntervalCb = std::function<void ()> ;
using MessageCb =  std::function<void (const char *from, int cmd,
        const char *data, size_t data_len,  std::vector<uint8_t> &data_binaty)>;

/**
 * Structure to reconnect room client to another L0 server (command CMD_R_RECONNECT)
 */
typedef struct roomClientReconnect {

    uint32_t port;      ///< Port number
    char data[];        ///< Contain string with: { char IP[], char peer_name[] }

} roomClientReconnect;

/**
 * Application parameters structure
 */
struct AppParameters {

    const char *client_name;
    const char *tcp_server;
    int tcp_port;
    const char *peer_name;
    const char *message;
    ConnectCb on_connect;
    DisconnectCb on_disconnect;
    MessageCb on_message;
    IntervalCb on_interval;
    Teo *cli;
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
                       "cmd = " << CMD_L_PEERS << " (CMD_L_L0_CLIENTS)\n";

                // Send (3) echo request to peer, command CMD_L_ECHO
                snd = cli.sendEcho(param->peer_name, param->message);
                std::cout << "Send " << snd << " bytes packet to L0 server to "
                       "peer " << param->peer_name << ", "
                       "cmd = " << CMD_L_PEERS << " (CMD_L_ECHO), "
                       "data: " << param->message << "\n";

                // Show empty line
                std::cout << "\n";
            }
            else std::cout << "Can't connect to server\n";

        } break;

        case EV_L_DISCONNECTED: {

            std::cout << "Disconnected\n";
            param->on_disconnect();

        } break;

        case EV_L_RECEIVED_UNRELIABLE: // fallthrough
        case EV_L_RECEIVED: {

            // Receive answer from server
            const size_t rc = data_len;
            teo::Packet *cp = cli.packet();

            std::cout << "Receive " << rc << " bytes: " << cp->data_length <<
                " bytes data from L0 server, "
                "from peer '" << cp->peer_name << "', cmd = " << int(cp->cmd) << "\n";


            const uint8_t *pointer = (const uint8_t*)cp->peer_name + cp->peer_name_length;
            std::vector<uint8_t> vec(pointer, pointer + cp->data_length);

            // JS on_message callback
            param->on_message(cp->peer_name, int(cp->cmd),
                    (const char*)(cp->peer_name + cp->peer_name_length),
                    cp->data_length, vec);

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
                } break;

                case CMD_L_ECHO: {

                    std::cout << "Got echo command\n";

                } break;

                // CMD_L_RECONNECT_TO_L0
                case 144: {
                    data = cp->peer_name + cp->peer_name_length;
                    auto d = (roomClientReconnect*)data;
                    std::cout << "Got 'Reconnect to another L0 server' command " 
                            << d->data << ":" << d->port 
                            << ", peer: '" << d->data + strlen(d->data) + 1 
                            << "'" << "\n";
                    cli.shutdown(); 
                } break;

                default: {

                    //std::cout << "Got unknown command\n";

                } break;
            }

        } break;

        case EV_L_IDLE: {
            if(param->on_interval) param->on_interval();
        } break;

        default:
            //std::cout << "Got unknown event: " << event << "\n";
            break;
    }
}

class Teo {

private:

    std::unique_ptr<teo::Teocli> cli;

public:

    Teo(const char *client_name, const char *tcp_server,
        int tcp_port, const char *peer_name, const char *message,
        ConnectCb on_connect, DisconnectCb on_disconnect, MessageCb on_message,
        IntervalCb on_interval = NULL, int interval = 1000) {

        std::cout << "Teo constructor\n";
            main_select(client_name, tcp_server, tcp_port, peer_name, message,
                on_connect, on_disconnect, on_message, on_interval, interval);
    }

public:

    /**
     * Send command to L0 server (to peer) with string data
     *
     * Create L0 clients packet and send it to L0 server
     *
     * @param cmd Command
     * @param peer_name Peer name to send to
     * @param data String with data
     *
     * @return Length of send data or -1 at error
     */
    inline ssize_t send(int cmd, const char *peer_name, const char *data) {
        std::cout << "Teo send, to: " << peer_name << "\n";
        return cli->send(cmd, peer_name, (void*)data, strlen(data) + 1);
    }

    /**
     * Send command to L0 server (to peer) with binary data
     *
     * Create L0 clients packet and send it to L0 server
     *
     * @param cmd Command
     * @param peer_name Peer name to send to
     * @param data Binary data
     *
     * @return Length of send data or -1 at error
     */
    inline ssize_t sendBinary(int cmd, const char *peer_name, std::vector<uint8_t> data) {
        std::cout << "Teo send, to: " << peer_name << "; data:";
        for(std::vector<uint8_t>::iterator it = data.begin(); it != data.end(); ++it) {
          std::cout << (int)*it << ",";
        }
        std::cout  << "; size: " << data.size() << "\n";
        uint8_t* pdata = &data[0];
        return cli->send(cmd, peer_name, (void*)pdata, data.size() * sizeof(uint8_t));
    }

    /**
     * Send echo command
     *
     * @param peer_name Peer name to send to
     * @param msg String with any text
     *
     * @return Length of send data or -1 at error
     */
    inline ssize_t sendEcho(const char *peer_name, const char*msg) {
        return cli->sendEcho(peer_name, msg);
    }

    /**
     * Send subscribe command to remote peer
     *
     * @param peer_name Peer name to send to
     * @param event Event number to subscribe to (should be more or equal to 0x8000)
     */
    inline void subscribe(const char *peer_name, uint16_t event) {
        cli->send(81/*CMD_SUBSCRIBE*/, peer_name, (void*)&event, sizeof(event));
    }

    /**
     * Send unsubscribe command to remote peer
     *
     * @param peer_name Peer name to send to
     * @param event Event number to unsubscribe to (should be more or equal to 0x8000)
     */
    inline void unsubscribe(const char *peer_name, uint16_t event) {
        cli->send(82/*CMD_USUBSCRIBE*/, peer_name, (void*)&event, sizeof(event));
    }

    /**
     * Get this client name
     *
     * @return  Reference to client name string
     */
    inline const std::string& getClientName() const {
        return cli->getClientName();
    }

    /**
     * Send reconnect to l0 command to Test Server which resend it to L0 client
     * 
     * @param peer_name
     * @param ip
     * @param port
     * @param peer
     * @return 
     */
    ssize_t sendReconnectToL0(const char *peer_name, const char *ip,
        uint32_t port, const char *peer) {
        // Prepare output data
        size_t ip_len = strlen(ip) + 1;
        size_t peer_len = strlen(peer) + 1;
        size_t out_data_len = ip_len + peer_len + sizeof(roomClientReconnect);
        roomClientReconnect *out_data = (roomClientReconnect *)malloc(out_data_len);
        out_data->port = port;
        memcpy(out_data->data, ip, ip_len);
        memcpy(out_data->data + ip_len, peer, peer_len);

        auto retval = cli->send(144/*CMD_L_RECONNECT_L0*/, peer_name, 
                (void*)out_data, out_data_len);
        
        free(out_data);

        return retval;
    }

private:

    AppParameters parameters;

    /**
     * Main L0 Native client example function
     *
     * @param argc Number of arguments
     * @param argv Arguments array
     *
     * @return
     */
    int main_select(const char *client_name, const char *tcp_server,
        int tcp_port, const char *peer_name, const char *message,
        ConnectCb on_connect, DisconnectCb on_disconnect, MessageCb on_message,
        IntervalCb on_interval, int interval) {

        // Welcome message
        std::cout << "Teonet c++ L0 client with Select and Event Loop Callback "
                "example version " TL0CN_VERSION " (Native TCP Client)\n\n";

        // Teonet L0 server parameters
        //static AppParameters
        parameters = (AppParameters) {
            client_name, // This client name
            tcp_server, // L0 tcp_server
            tcp_port, // L0 tcp_port
            peer_name, // Peer name to send test messages to
            message, // Test message
            on_connect, // On connect callback
            on_disconnect, // On disconnect callback
            on_message, // On message callback
            on_interval,
            this
        };

        // Create Teocli object, Initialize L0 Client library and connect to L0 server
        cli = std::make_unique<teo::Teocli>(parameters.client_name,
            parameters.tcp_server, parameters.tcp_port, event_cb, &parameters);

        // Call JS "on connect" callback function with type 0 - initialized
        on_connect(0, this);

        auto reconnect = true; // Reconnect this client if disconnected
        unsigned long num = 0; // An index
        int attempt = 0; // Reconnect attempt count

        while(reconnect) {

            if(cli->connected() > 0) {
                
                attempt = 0;

                // Call JS "on connect" callback function with type 0 - initialized
                on_connect(1, this);

                #ifdef __EMSCRIPTEN__
                emscripten_set_main_loop_arg(cli->eventLoopE, cli.get(), 60, 0);
                #else
                // Event loop
                while(cli->eventLoop(interval)) {

                    // Send Echo command every second
                    //if( !(num % (1000 / timeout)) )
                    //    cli.sendEcho(parameters.peer_name, parameters.message);

                    num++;
                }
                std::cout << "Exit from event loop\n";
                cli->disconnect();
                #endif
            }
            else {
                std::cout << "Try to reconnect...\n";
                if(attempt++) cli->sleep(1000);
                cli->connect(tcp_server, tcp_port, &parameters, event_cb);
            }
        }

        return (EXIT_SUCCESS);
    }
};

#include "nbind/nbind.h"

NBIND_CLASS(Teo) {

  construct<const char *, const char *, int , const char *, const char *,
        ConnectCb, DisconnectCb, MessageCb, IntervalCb>();
  construct<const char *, const char *, int , const char *, const char *,
        ConnectCb, DisconnectCb, MessageCb, IntervalCb, int >();

  method(send);
  method(sendBinary);
  method(sendEcho);
  method(subscribe);
  method(unsubscribe);
  method(getClientName);
  method(sendReconnectToL0);
}
