/** 
 * File:   teonet_lo_client.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on October 12, 2015, 12:32 PM
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#if !(defined(_WIN32) || defined(_WIN64))
#include <unistd.h>
#endif
#include <fcntl.h>
#if defined(_WIN32) || defined(_WIN64) 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define BARNABY_API __declspec(dllexport )
#include <winsock2.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#include <sys/timeb.h> 

#include "teonet_l0_client.h"

// Uncomment next line to show debug message
//#define CONNECT_MSG
//#define DEBUG_MSG

// Internal functions
static ssize_t teoLNullPacketSend(int sd, void* pkg, size_t pkg_length);
static ssize_t teoLNullPacketRecv(int sd, void* buf, size_t buf_length);
static ssize_t teoLNullPacketSplit(teoLNullConnectData *con, void* data,
        size_t data_len, ssize_t received);


#if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64) 
    #define close_socket(sd) closesocket(sd)
#else
    #define close_socket(sd) close(sd)
#endif

#if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64) 
void TEOCLI_API WinSleep(uint32_t dwMilliseconds) {Sleep(dwMilliseconds);}
#endif

// Send connected event
#define send_l0_event(con, event, data, data_length) \
    if(con->event_cb != NULL) { \
        con->event_cb(con, event, data, data_length, con->user_data); \
    }

/**
 * Initialize L0 client library.
 * 
 * Startup windows socket library.
 * Calls once per application to initialize this client library.
 */
void teoLNullInit() {
    
    // Startup windows socket library
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);
    #endif     
}


/**
 * Cleanup L0 client library.
 * 
 * Cleanup windows socket library.
 * Calls once per application to cleanup this client library.
 */
void teoLNullCleanup() {
    
    // Cleanup windows socket library
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    WSACleanup();
    #endif    
}

/**
 * Create L0 client packet
 *  
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param command Command to peer
 * @param peer Teonet peer
 * @param data Command data
 * @param data_length Command data length
 * 
 * @return Length of created packet or zero if buffer to less
 */
size_t teoLNullPacketCreate(void* buffer, size_t buffer_length, 
        uint8_t command, const char * peer, const void* data, 
        size_t data_length) {
    
    size_t peer_name_length = strlen(peer) + 1;
    
    // Check buffer length
    if(buffer_length < sizeof(teoLNullCPacket) + peer_name_length + data_length)           
        return 0;
    
    teoLNullCPacket* pkg = (teoLNullCPacket*) buffer;
    memset(buffer, 0, sizeof(teoLNullCPacket));
    
    pkg->cmd = command;
    pkg->data_length = (uint16_t)data_length;
	pkg->peer_name_length = (uint8_t)peer_name_length;
    memcpy(pkg->peer_name, peer, pkg->peer_name_length);
    memcpy(pkg->peer_name + pkg->peer_name_length, data, pkg->data_length);
    pkg->checksum = get_byte_checksum(pkg->peer_name, pkg->peer_name_length + 
            pkg->data_length);
    pkg->header_checksum = get_byte_checksum(pkg, sizeof(teoLNullCPacket) - 
            sizeof(pkg->header_checksum));
    
    return sizeof(teoLNullCPacket) + pkg->peer_name_length + pkg->data_length;
}

/**
 * Send packet to L0 server
 * 
 * @param sd L0 server socket
 * @param pkg Package to send
 * @param pkg_length Package length
 * 
 * @return Length of send data or -1 at error
 */
static ssize_t teoLNullPacketSend(int sd, void* pkg, size_t pkg_length) {
    
	ssize_t snd;
    
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
	if ((snd = send(sd, pkg, (int)pkg_length, 0)) >= 0);
    #else
    if((snd = write(sd, pkg, pkg_length)) >= 0);                
    #endif

    return snd;
}

/**
 * Send command to L0 server
 * 
 * Create L0 clients packet and send it to L0 server
 * 
 * @param con Pointer to teoLNullConnectData
 * @param cmd Command
 * @param peer_name Peer name to send to
 * @param data Pointer to data
 * @param data_length Length of data
 * 
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullSend(teoLNullConnectData *con, int cmd, const char *peer_name, 
        void *data, size_t data_length) {
    
    if(data == NULL) data_length = 0;
    
    const size_t peer_length = strlen(peer_name) + 1;
    const size_t buf_length = teoLNullBufferSize(peer_length, data_length);
    char *buf = malloc(buf_length);
    ssize_t snd;
    
    size_t pkg_length = teoLNullPacketCreate(buf, buf_length, cmd, peer_name, 
            data, data_length);
    if((snd = teoLNullPacketSend((int)con->fd, buf, pkg_length)) >= 0);
    
    free(buf);
    
    return snd;
}

static size_t teo_time_length() {
    
    return sizeof(struct timeval);
}

static void *teo_time_get(size_t *time_size) {

	size_t len = teo_time_length();
	struct timeval *tv = malloc(len);
	gettimeofday(tv, 0);
	if(time_size) *time_size = len;

	return tv;
}

static void teo_time_free(void *tv) {
	free(tv);
}

static int teo_time_diff(void *tv) {

	struct timeval *tv_last = tv,
		       *tv_current = teo_time_get(0);
	int ret = (tv_current->tv_sec - tv_last->tv_sec) * 1000
		+ (tv_current->tv_usec - tv_last->tv_usec) / 1000;
	teo_time_free(tv_current);

	return ret;
};

/**
 * Send ECHO command to L0 server
 * 
 * Create L0 clients packet and send it to L0 server
 * 
 * @param con Pointer to teoLNullConnectData
 * @param peer_name Peer name to send to
 * @param msg Message
 * 
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullSendEcho(teoLNullConnectData *con, const char *peer_name,
        const char *msg) {
    
    // Add current time to the end of message (it should be return 
    // back by server)
    
//    struct timeb time_start;
//    const size_t time_length = sizeof(struct timeb);
//    ftime(&time_start);
    
    // Get current time in millisecond
    size_t time_length;
    void *time_start = teo_time_get(&time_length);
    
    const size_t msg_len = strlen(msg) + 1;
    const size_t msg_buf_len = msg_len + time_length;
    char *msg_buf = malloc(msg_buf_len); 
    //
    // Fill message buffer
    memcpy(msg_buf, msg, msg_len);
    memcpy(msg_buf + msg_len, time_start, time_length);
    //
    // Send message with time
    ssize_t snd = teoLNullSend(con, CMD_L_ECHO, peer_name, msg_buf, 
            msg_buf_len);
    
    teo_time_free(time_start);
    free(msg_buf);
    
    return snd;
}

/**
 * Process ECHO_ANSWER request
 * 
 * @param msg Echo answers command data
 * @return Trip time in ms
 */
int teoLNullProccessEchoAnswer(const char *msg) {
    
    //struct timeb time_start, time_end;
    void *time_start;
    size_t time_length = teo_time_length() ; // = sizeof(struct timeb);
    
    // Get time from answers data
    //ftime(&time_end);
    //time_end = teo_time_get(&time_length);
    size_t time_ptr = strlen(msg) + 1;
    //memcpy(time_start, msg + time_ptr, time_length);
    time_start = (void *)msg + time_ptr;

    // Calculate trip time
    int trip_time = 
//            (int) (1000.0 * (time_end.time - time_start.time)
//            + (time_end.millitm - time_start.millitm));
        teo_time_diff(time_start);

    return trip_time;
}

/**
 * Split or Combine input buffer
 * 
 * @param kld Pointer to input buffer
 * @param data Received data buffer
 * @param data_len Received data buffer length
 * @param received Received data length
 * 
 * @return Size of packet or Packet state code
 * @retval >0 Packet received
 * @retval -1 Packet not receiving yet (got part of packet)
 * @retval -2 Wrong packet received (dropped)
 */
static ssize_t teoLNullPacketSplit(teoLNullConnectData *kld, void* data, 
        size_t data_len, ssize_t received) {
    
    ssize_t retval = -1;
    
    #ifdef DEBUG_MSG
    printf("L0 Client: "
       "Got %d bytes of packet...\n", 
       (int)received);        
    #endif
    
    // Check end of previous buffer
    if(kld->last_packet_ptr > 0) {

        kld->read_buffer_ptr = kld->read_buffer_ptr - kld->last_packet_ptr;
        if(kld->read_buffer_ptr > 0) {
            
            #ifdef DEBUG_MSG
            printf("L0 Client: "
                   "Use %d bytes from previously received data...\n", 
                   (int)(kld->read_buffer_ptr));
            #endif
                    
            memmove(kld->read_buffer, (char*)kld->read_buffer + 
                    kld->last_packet_ptr, kld->read_buffer_ptr);
        }        
        kld->last_packet_ptr = 0;
    }    
                
    // Increase buffer size
    if((size_t)received > kld->read_buffer_size - kld->read_buffer_ptr) {

        // Increase read buffer size
        kld->read_buffer_size += data_len; //received;
        if(kld->read_buffer != NULL) 
            kld->read_buffer = realloc(kld->read_buffer, kld->read_buffer_size);
        else 
            kld->read_buffer = malloc(kld->read_buffer_size);     

        #ifdef DEBUG_MSG
        printf("L0 Client: "
               "Increase read buffer to new size: %d bytes ...\n", 
               (int)kld->read_buffer_size);        
        #endif
    }
    
    // Add received data to the read buffer
    if(received > 0) {
        memmove((char*)kld->read_buffer + kld->read_buffer_ptr, data, received);
        kld->read_buffer_ptr += received;
    }

    teoLNullCPacket *packet = (teoLNullCPacket *)kld->read_buffer;
    ssize_t len;

    // \todo Check packet

    // Process read buffer
    if(kld->read_buffer_ptr - kld->last_packet_ptr > sizeof(teoLNullCPacket) && 
       kld->read_buffer_ptr - kld->last_packet_ptr >= 
            (size_t)(len = sizeof(teoLNullCPacket) + packet->peer_name_length + 
            packet->data_length)) {

        // Check checksum
        uint8_t header_checksum = get_byte_checksum(packet, 
                sizeof(teoLNullCPacket) - 
                sizeof(packet->header_checksum));
        uint8_t checksum = get_byte_checksum(packet->peer_name, 
                packet->peer_name_length + packet->data_length);
        if(packet->header_checksum == header_checksum &&
           packet->checksum == checksum) {

            // Packet has received - return packet size
            retval = len; 
            kld->last_packet_ptr += len;                    

            #ifdef DEBUG_MSG
            printf("L0 Server: "
                "Identify packet %d bytes length ...\n", 
                (int)retval);
            #endif
        }

        // Wrong checksum - drop this packet
        else {

            // Wrong packet received - return -2
            kld->read_buffer_ptr = 0;
            kld->last_packet_ptr = 0;
            retval = -2;

            #ifdef DEBUG_MSG
            printf("L0 Client: "
                "Wrong packet %d bytes length; dropped ...\n", 
                (int)len);
            #endif
        }
    }
    else {
        
        #ifdef DEBUG_MSG
        printf("L0 Client: "
               "Wait next part of packet, now it has %d bytes ...\n", 
               (int)kld->read_buffer_ptr);        
        #endif
    }

    return retval;    
}

/**
 * Receive packet from L0 server
 * 
 * @param sd L0 server socket
 * @param buf Buffer to receive
 * @param buf_length Buffer length
 * 
 * @return Length of send data
 */
static ssize_t teoLNullPacketRecv(int sd, void* buf, size_t buf_length) {
    
	ssize_t rc;
    
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    rc = recv(sd, buf, (int)buf_length, 0);
    #else
    rc = read(sd, buf, buf_length);
    #endif

    return rc;
}

/**
 * Receive packet from L0 server and split or combine it
 * 
 * @param con Pointer to teoLNullConnectData
 * 
 * @return Size of packet or Packet state code
 * @retval >0 Packet received
 * @retval  0 Disconnected
 * @retval -1 Packet not receiving yet (got part of packet)
 * @retval -2 Wrong packet received (dropped)
 */
ssize_t teoLNullRecv(teoLNullConnectData *con) {
    
    char buf[L0_BUFFER_SIZE];
    
    ssize_t rc = teoLNullPacketRecv((int)con->fd, buf, L0_BUFFER_SIZE);
    if(rc != 0) {
        rc = teoLNullPacketSplit(con, buf, L0_BUFFER_SIZE, rc != -1 ? rc : 0);
     
        // Send echo answer to echo command
        if(rc > 0) {
            teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
            if(cp->cmd == CMD_L_ECHO) {
                char *data = cp->peer_name + cp->peer_name_length;
                teoLNullSend(con, CMD_L_ECHO_ANSWER, cp->peer_name, data, cp->data_length );
            }
        }
    }
    
    return rc;
}

/**
 * Create L0 clients initialization packet
 * 
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param host_name Name of this L0 client
 * 
 * @return Length of created packet or zero if buffer to less
 */
size_t teoLNullPacketCreateLogin(void* buffer, size_t buffer_length, 
        const char* host_name) {
    
    return teoLNullPacketCreate(buffer, buffer_length, 0, "", host_name, 
            strlen(host_name) + 1);
}

/**
 * Login to L0 server
 * 
 * Create and send L0 clients initialization packet
 * 
 * @param con Pointer to teoLNullConnectData
 * @param host_name Client name
 * 
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullLogin(teoLNullConnectData *con, const char* host_name) {

    ssize_t snd;
    const size_t buf_len = teoLNullBufferSize(1, strlen(host_name) + 1);
    
    // Buffer
    #if defined(_WIN32) || defined(_WIN64)
    char *buf = malloc(buf_len);
    #else
    char buf[buf_len];
    #endif
    
    size_t pkg_length = teoLNullPacketCreateLogin(buf, buf_len, host_name);
    if(!pkg_length) return 0;
	if ((snd = teoLNullPacketSend((int)con->fd, buf, pkg_length)) >= 0);
    
    // Free buffer
    #if defined(_WIN32) || defined(_WIN64)
    free(buf);
    #endif
    
    return snd;
}

/**
 * Calculate checksum
 * 
 * Calculate byte checksum in data buffer
 * 
 * @param data Pointer to data buffer
 * @param data_length Length of the data buffer to calculate checksum
 * 
 * @return Byte checksum of the input buffer
 */
uint8_t get_byte_checksum(void *data, size_t data_length) {
    
    int i;
    uint8_t *ch, checksum = 0;
    for(i = 0; i < (int)data_length; i++) {
        
        ch = (uint8_t*)((char*)data + i);
        checksum += *ch;
    }
    
    return checksum;
}

/**
 * Set socket or SD to non blocking mode
 * 
 * @param sd Socket descriptor
 */
void set_nonblock(int sd) {

    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    int iResult;
    u_long iMode = 1;

    iResult = ioctlsocket(sd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
      printf("ioctlsocket failed with error: %ld\n", iResult);
    
    #else
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    fcntl(sd, F_SETFL, flags | O_NONBLOCK);
    #endif
}

/**
 * Set TCP NODELAY option
 * 
 * @param sd TCP socket descriptor
 * 
 * @return Result of setting. Success if >= 0.
 */
int set_tcp_nodelay(int sd) {

    int result = 0;    
    int flag = 1;
    
    result = setsockopt(sd,           // socket affected
                        IPPROTO_TCP,     // set option at TCP level
                        TCP_NODELAY,     // name of option
                        (char *) &flag,  // the cast is historical cruft
                        sizeof(flag));   // length of option value
    if (result < 0) {
        
        printf("Set TCP_NODELAY of sd %d error\n", sd);
    }

    return result;
}

/**
 * Wait socket data during timeout and call callback if data received
 * 
 * @param con Pointer to teoLNullConnectData
 * @param timeout Timeout of wait socket read event in ms
 * 
 * @return 0 - if disconnected or 1 other way
 */
int teoLNullReadEventLoop(teoLNullConnectData *con, int timeout) {
    
    int rv, retval = 1;
    fd_set rfds;
    struct timeval tv;

    // Watch server_socket to see when it has input.
    FD_ZERO(&rfds);
    FD_SET(con->fd, &rfds);

    // Wait up to 50 ms. */
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

	rv = select((int)con->fd + 1, &rfds, NULL, NULL, &tv);
    
    // Error
    if (rv == -1) printf("select() handle error\n");
    
    // Timeout
    else if(!rv) { // Idle or Timeout event

        send_l0_event(con, EV_L_IDLE, NULL, 0);
    }
    // There is a data in sd
    else {
        
        //printf("Data in sd\n");
        ssize_t rc;
        while((rc = teoLNullRecv(con)) != -1) {
            
            if(rc > 0) {
                send_l0_event(con, EV_L_RECEIVED, con->read_buffer, rc);
            } else if(rc == 0) {
                send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
                retval = 0;
                break;
            }
        }
    }
    
    // Send Tick event    
    send_l0_event(con, EV_L_TICK, NULL, 0);
    
    return retval;
}

/**
 * Create TCP client and connect to server with event callback
 * 
 * @param server Server IP or name
 * @param port Server port
 * @param event_cb Pointer to event callback function
 * @param user_data Pointer to user data which will be send to event callback
 * 
 * @return Pointer to teoLNullConnectData. Null if no memory error
 * @retval teoLNullConnectData::fd>0   - Success connection
 * @retval teoLNullConnectData::fd==-1 - Create socket error
 * @retval teoLNullConnectData::fd==-2 - HOST NOT FOUND error
 * @retval teoLNullConnectData::fd==-3 - Client-connect() error
 */
teoLNullConnectData* teoLNullConnectE(const char *server, int port, 
        teoLNullEventsCb event_cb, void *user_data) {
    
    // Variable and structure definitions.
    int rc;
    struct hostent *hostp;
    struct sockaddr_in serveraddr;
    teoLNullConnectData *con = malloc(sizeof(teoLNullConnectData));
    if(con == NULL) return con;
    
    con->last_packet_ptr = 0;
    con->read_buffer = NULL;
    con->read_buffer_ptr = 0;
    con->read_buffer_size = 0;
    con->event_cb = event_cb;
    con->user_data = user_data;

    /* The socket() function returns a socket */
    /* descriptor representing an endpoint. */
    /* The statement also identifies that the */
    /* INET (Internet Protocol) address family */
    /* with the TCP transport (SOCK_STREAM) */
    /* will be used for this socket. */
    /******************************************/
    /* get a socket descriptor */
    if((con->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    
        printf("Client-socket() error\n");
        con->fd = -1;
        send_l0_event(con, EV_L_CONNECTED, &con->fd, sizeof(con->fd));
        return con;
    }
    else {
        #ifdef DEBUG_MSG
        printf("Client-socket() OK\n");
        #endif
    }

    #ifdef CONNECT_MSG
    printf("Connecting to the server %s at port %d ...\n", server, port);
    #endif

    memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    if((serveraddr.sin_addr.s_addr = inet_addr(server)) == 
                (unsigned long)INADDR_NONE) {

        /* When passing the host name of the server as a */
        /* parameter to this program, use the gethostbyname() */
        /* function to retrieve the address of the host server. */
        /***************************************************/
        /* get host address */
        hostp = gethostbyname(server);
        if(hostp == (struct hostent *)NULL) {
        
            printf("HOST NOT FOUND --> h_errno = %d\n", h_errno);
			close_socket(con->fd);
            con->fd = -2;
            send_l0_event(con, EV_L_CONNECTED, &con->fd, sizeof(con->fd));
            return con;
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr_list[0], sizeof(serveraddr.sin_addr));
    }

    /* After the socket descriptor is received, the */
    /* connect() function is used to establish a */
    /* connection to the server. */
    /***********************************************/
    /* connect() to server. */
    if((rc = connect(con->fd, (struct sockaddr *)&serveraddr, 
            sizeof(serveraddr))) < 0) {
    
        printf("Client-connect() error\n");
		close_socket(con->fd);
        con->fd = -3;        
        send_l0_event(con, EV_L_CONNECTED, &con->fd, sizeof(con->fd));
        return con;
    }
    else {
        #ifdef CONNECT_MSG
        printf("Connection established ...\n");
        #endif
    }

    // Set non block mode
    set_nonblock((int)con->fd);
    
    // Set TCP_NODELAY option
    set_tcp_nodelay((int)con->fd);
    
    // Send connected event
    send_l0_event(con, EV_L_CONNECTED, &con->fd, sizeof(con->fd));

    return con;
}

/**
 * Create TCP client and connect to server without event callback
 * 
 * @param server Server IP or name
 * @param port Server port
 * 
 * @return Pointer to teoLNullConnectData. Null if no memory error
 * @retval teoLNullConnectData::fd>0   - Success connection
 * @retval teoLNullConnectData::fd==-1 - Create socket error
 * @retval teoLNullConnectData::fd==-2 - HOST NOT FOUND error
 * @retval teoLNullConnectData::fd==-3 - Client-connect() error
 */
teoLNullConnectData* teoLNullConnect(const char *server, int port) {
    
    return teoLNullConnectE(server, port, NULL, NULL);
}

/**
 * Disconnect from server and free teoLNullConnectData
 * 
 * @param con Pointer to teoLNullConnectData
 */
void teoLNullDisconnect(teoLNullConnectData *con) {
    
    if(con != NULL) {
        
        if(con->fd > 0) close_socket(con->fd);
        if(con->read_buffer != NULL) free(con->read_buffer);
        free(con);
    }
}

#undef DEBUG_MSG
