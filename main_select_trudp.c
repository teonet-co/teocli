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
#include <stdarg.h>

#include "libteol0/teonet_l0_client.h"
#include "../trudp/src/trudp.h"
#include "../trudp/src/utils.h"

#define DEBUG 1
#define TL0CNS_VERSION "0.0.2"

// Application constants
#define SEND_MESSAGE_AFTER_MIN  15000 /* 16667 */ // uSec (mSec * 1000)
#define SEND_MESSAGE_AFTER  1000000
#define RECONNECT_AFTER 3000000 // uSec (mSec * 1000)
#define SHOW_STATISTIC_AFTER 500000 // uSec (mSec * 1000)


/**
 * Show error and exit
 *
 * @param fmt
 * @param ...
 */
static void die(char *fmt, ...)
{
	va_list ap;
	fflush(stdout);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * Show debug message
 *
 * @param fmt
 * @param ...
 */
static void debug(char *fmt, ...)
{
    static unsigned long idx = 0;
    va_list ap;
    if(DEBUG) {
        fflush(stdout);
        fprintf(stderr, "%lu %.3f debug: ", ++idx, trudpGetTimestamp() / 1000.0);
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fflush(stderr);
    }
}

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

                case CMD_L_ECHO_ANSWER:
                {
                    printf("Got echo answer command\n");
                    data = cp->peer_name + cp->peer_name_length;
                    int trip_time = teoLNullProccessEchoAnswer(data);

                    // Show data
                    printf("Data: %s\n", (char*)data);

                    // Show trip time
                    printf("Trip time: %d ms\n\n", trip_time);

                } break;

                case CMD_L_ECHO:
                {
                    printf("Got echo command\n");

                } break;
                
                case CMD_L_AUTH_LOGIN_ANSWER: {
                    
                    printf("Got answer from authentication server\n");
                    
                    const char *auth_data = (const char *)
                            (cp->peer_name + cp->peer_name_length);
                    
                    // Show data
                    printf("Data: %s\n\n", auth_data);
                }
                break;


                default:
                    printf("Got unknown command\n");
                    break;
            }

        } break;

        default:
            break;
    }
}

static int connected_flag = 0;
static char *remote_address;
static int remote_port_i;



/**
 * TR-UDP event callback
 *
 * @param tcd_pointer
 * @param event
 * @param data
 * @param data_length
 * @param user_data
 */
static void event_cb_trudp(void *tcd_pointer, int event, void *data, 
        size_t data_length, void *user_data) {

    trudpChannelData *tcd = (trudpChannelData *)tcd_pointer;

    switch(event) {

        // CONNECTED event
        // @param data NULL
        // @param user_data NULL
        case CONNECTED: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr, "Connect channel %s\n", key);

        } break;

        // DISCONNECTED event
        // @param tcd Pointer to trudpData
        // @param data Last packet received
        // @param user_data NULL
        case DISCONNECTED: {
                        
            char *key = trudp_ChannelMakeKey(tcd);
            if(data_length == sizeof(uint32_t)) {
                uint32_t last_received = *(uint32_t*)data;
                fprintf(stderr,
                    "Disconnect channel %s, last received: %.6f sec\n",
                    key, last_received / 1000000.0);
                trudp_ChannelDestroy(tcd);
            }
            else {
                fprintf(stderr,
                    "Disconnected channel %s\n", key);
            }

            connected_flag = 0;

        } break;

        // GOT_RESET event
        // @param data NULL
        // @param user_data NULL
        case GOT_RESET: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr,
              "Got TRU_RESET packet from channel %s\n",
              key);
            
            connected_flag = 0;

        } break;

        // SEND_RESET event
        // @param data Pointer to uint32_t id or NULL (data_size == 0)
        // @param user_data NULL
        case SEND_RESET: {


            char *key = trudp_ChannelMakeKey(tcd);
            
            if(!data)
                fprintf(stderr,
                  "Send reset: "
                  "to channel %s\n",
                  key);

            else {
            
                uint32_t id = (data_length == sizeof(uint32_t)) ? *(uint32_t*)data:0;

                if(!id)
                    fprintf(stderr,
                      "Send reset: "
                      "Not expected packet with id = 0 received from channel %s\n",
                      key);
                else
                    fprintf(stderr,
                      "Send reset: "
                      "High send packet number (%d) at channel %s\n",
                      id, key);
                }

        } break;

        // GOT_ACK_RESET event: got ACK to reset command
        // @param data NULL
        // @param user_data NULL
        case GOT_ACK_RESET: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr, "Got ACK to RESET packet at channel %s\n", key);

        } break;

        // GOT_ACK_PING event: got ACK to ping command
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_ACK_PING: {

            char *key = trudp_ChannelMakeKey(tcd);
            //fprintf(stderr,
            debug(
              "Got ACK to PING packet at channel %s, data: %s, %.3f(%.3f) ms\n",
              key, (char*)data,
              (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

        } break;

        // GOT_PING event: got PING packet, data
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_PING: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr,
              "Got PING packet at channel %s, data: %s\n",
              key, (char*)data);

        } break;

        // Got ACK event
        // @param data Pointer to ACK packet
        // @param data_length Length of data
        // @param user_data NULL
        case GOT_ACK: {

            char *key = trudp_ChannelMakeKey(tcd);
            debug("Got ACK id=%u at channel %s, %.3f(%.3f) ms\n",
                  trudpPacketGetId(data/*trudpPacketGetPacket(data)*/),
                  key, (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

            #if USE_LIBEV
            // trudp_start_send_queue_cb(&psd, 0);
            #endif

        } break;

        // Got DATA event
        // @param data Pointer to data
        // @param data_length Length of data
        // @param user_data NULL
        case GOT_DATA: {

            char *key = trudp_ChannelMakeKey(tcd);
            
            teoLNullCPacket *cp = trudpPacketGetData(trudpPacketGetPacket(data));
            
            debug("Got %d byte data at channel %s, id=%u, from: %s, data: %s\n", 
                trudpPacketGetPacketLength(trudpPacketGetPacket(data)),
                key, 
                trudpPacketGetId(trudpPacketGetPacket(data)), 
                cp->peer_name,
                cp->peer_name + cp->peer_name_length );

//            if(!o.show_statistic && !o.show_send_queue && !o.show_snake) {
//                if(o.debug) {
//                    printf("#%u at %.3f, cannel %s [%.3f(%.3f) ms] ",
//                           tcd->receiveExpectedId,
//                           (double)trudpGetTimestamp() / 1000.0,
//                           key, 
//                           (double)tcd->triptime / 1000.0,
//                           (double)tcd->triptimeMiddle / 1000.0);
//
//                    printf("%s\n",(char*)data);
//                }
//            }
//            else {
//                // Show statistic window
//                //showStatistic(TD(tcd));
//            }
            
            //debug("%s\n", " ");

        } break;
        
        // Process received data
        // @param tcd Pointer to trudpData
        // @param data Pointer to receive buffer
        // @param data_length Receive buffer length
        // @param user_data NULL
        case PROCESS_RECEIVE: {
            
            trudpData *td = (trudpData *)tcd;
            trudpProcessReceive(td, data, data_length);
                        
        } break;
        
        // Process send data
        // @param data Pointer to send data
        // @param data_length Length of send
        // @param user_data NULL
        case PROCESS_SEND: {
        
            //if(isWritable(TD(tcd)->fd, timeout) > 0) {
            // Send to UDP
            trudpUdpSendto(TD(tcd)->fd, data, data_length,
                    (__CONST_SOCKADDR_ARG) &tcd->remaddr, sizeof(tcd->remaddr));
            //}

            // Debug message
            if(DEBUG) {

                int port,type;
                uint32_t id = trudpPacketGetId(data);
                char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
                if(!(type = trudpPacketGetType(data))) {
                    debug("Send %d bytes, id=%u, to %s:%d, %.3f(%.3f) ms\n",
                        (int)data_length, id, addr, port,
                        tcd->triptime / 1000.0, tcd->triptimeMiddle / 1000.0);
                }
                else {
                    debug("Send %d bytes %s id=%u, to %s:%d\n",
                        (int)data_length, 
                        type == 1 ? "ACK" :
                        type == 2 ? "RESET" :
                        type == 3 ? "ACK to RESET" :
                        type == 4 ? "PING" : "ACK to PING"
                        , id, addr, port);
                }
            }

            #if USE_LIBEV
            trudp_start_send_queue_cb(&psd, 0);
            #endif
            
        } break;

        default: break;
    }
}

// Read buffer
static char *buffer;
static const int BUFFER_SIZE = 2048;

/**
 * The TR-UDP cat network loop with select function
 *
 * @param td Pointer to trudpData
 * @param delay Default read data timeout
 */
static void network_select_loop(trudpData *td, int timeout) {

    int rv = 1;
    fd_set rfds, wfds;
    struct timeval tv;
    uint64_t /*tt, next_et = UINT64_MAX,*/ ts = trudpGetTimestampFull();

    // Watch server_socket to see when it has input.
    FD_ZERO(&wfds);
    FD_ZERO(&rfds);
    FD_SET(td->fd, &rfds);

    // Process write queue
    if(trudp_WriteQueueSizeAll(td)) {
        FD_SET(td->fd, &wfds);
    }

    uint32_t timeout_sq = trudp_SendQueueGetTimeout(td, ts);

    // Wait up to ~50 ms. */
    uint32_t t = timeout_sq < timeout ? timeout_sq : timeout;
    usecToTv(&tv, t);

    rv = select((int)td->fd + 1, &rfds, &wfds, NULL, &tv);

    // Error
    if (rv == -1) {
        fprintf(stderr, "select() handle error\n");
        return;
    }

    // Timeout
    else if(!rv) { // Idle or Timeout event

        // Process send queue
        if(timeout_sq != UINT32_MAX) {
            int rv = trudp_SendQueueProcess(td, 0);
            if(rv) debug("Process send queue ... %d\n", rv);
        }
    }

    // There is a data in fd
    else {

        // Process read fd
        if(FD_ISSET(td->fd, &rfds)) {

            struct sockaddr_in remaddr; // remote address
            socklen_t addr_len = sizeof(remaddr);
            ssize_t recvlen = trudpUdpRecvfrom(td->fd, buffer, BUFFER_SIZE,
                    (__SOCKADDR_ARG)&remaddr, &addr_len);

            // Process received packet
            if(recvlen > 0) {
                size_t data_length;
                trudpChannelData *tcd = trudpGetChannelCreate(td, (__SOCKADDR_ARG)&remaddr, 0);
                trudp_ChannelProcessReceivedPacket(tcd, buffer, recvlen, &data_length);
            }
        }

        // Process write fd
        if(FD_ISSET(td->fd, &wfds)) {
            // Process write queue
            while(trudp_WriteQueueProcess(td));
        }
    }
}

/**
 * Connect (login) to peer
 *
 * @param td
 * @return
 */
static trudpChannelData *connectToPeer(trudpData *td, const char * host_name) {

    trudpChannelData *tcd = NULL;
    
    ssize_t snd;
    const size_t buf_len = teoLNullBufferSize(1, strlen(host_name) + 1);

    // Buffer
    #if defined(_WIN32) || defined(_WIN64)
    char *buf = malloc(buf_len);
    #else
    char buf[buf_len];
    #endif

    size_t pkg_length = teoLNullPacketCreateLogin(buf, buf_len, host_name);
    if(!pkg_length) return NULL;
    
    tcd = trudp_ChannelNew(td, remote_address, remote_port_i, 0);
    trudp_ChannelSendData(tcd, buf, pkg_length);
    fprintf(stderr, "Connecting to %s:%u:%u\n", remote_address, remote_port_i, 0);
    connected_flag = 1;
    
    //if ((snd = teoLNullPacketSend((int)con->fd, buf, pkg_length)) >= 0) {};

    // Free buffer
    #if defined(_WIN32) || defined(_WIN64)
    free(buf);
    #endif

//    // Start connection and Send "connect" packet
//    char *connect = "Connect with TR-UDP!";
//    size_t connect_length = strlen(connect) + 1;
//    tcd = trudp_ChannelNew(td, remote_address, remote_port_i, 0);
//    trudp_ChannelSendData(tcd, connect, connect_length);
//    fprintf(stderr, "Connecting to %s:%u:%u\n", remote_address, remote_port_i, 0);
//    connected_flag = 1;

    return tcd;
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
    printf("Teonet L0 client with Select and Event Loop Callback example "
           "version " TL0CN_VERSION " (Native TCP/UDP Client)\n\n");

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
    
    // Connect to L0 TR-UDP server
    // Bind UDP port and get FD (start listening at port)
    int port = 9090; //atoi(o_local_port);
    int fd = trudpUdpBindRaw(&port, 1);
    if(fd <= 0) die("Can't bind UDP port ...\n");
    else fprintf(stderr, "Start listening at UDP port %d\n", port);

    // Initialize TR-UDP
    remote_address = (char*)param.tcp_server;
    remote_port_i = param.tcp_port;
    trudpData *td = trudpInit(fd, port, event_cb_trudp, NULL);
    if(td->fd > 0) {
        
        printf("TR-UDP port created\n");
        
        // Create read buffer
        buffer = malloc(BUFFER_SIZE);

        uint32_t tt, tt_s = 0, tt_c = 0, tt_ss = 0;
        const int DELAY = 500000; // uSec
        unsigned long num = 0;
        int quit_flag = 0;
        
        char *message;
        size_t message_length;
        message = "hello_c"; 
        message_length = strlen(message) + 1;

        trudpChannelData *tcd = NULL;
                
        // Event loop
        while(!quit_flag) {

            network_select_loop(td, SEND_MESSAGE_AFTER < DELAY ? SEND_MESSAGE_AFTER : DELAY);
            
            // Current timestamp
            tt = trudpGetTimestamp();

            // Connect
            if(!connected_flag && (tt - tt_c) > RECONNECT_AFTER) {
                tcd = connectToPeer(td, param.host_name);
                tt_c = tt;
            }
            
            if(connected_flag) {
                if((tt - tt_s) > SEND_MESSAGE_AFTER*5) {

                    char buf[BUFFER_SIZE];

                    size_t pkg_length = teoLNullPacketCreateEcho(buf, BUFFER_SIZE, param.peer_name, param.msg);
                    trudp_ChannelSendData(tcd, buf, pkg_length);

                    tt_s = tt;
                }
                else {
                    trudpProcessKeepConnection(td);
                }
            }
            

            num++;
        }
        
        // Destroy TR-UDP
        trudpDestroy(td);         
        free(buffer);
    }
    
    
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
