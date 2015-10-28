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
#else
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include "teonet_l0_client.h"

// Uncomment next line to show debug message
//#define DEBUG_MSG

#if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64) 
    #define close_socket(fd) closesocket(fd)
#else
    #define close_socket(fd) close(fd)
#endif

/**
 * Get output buffer size
 * @param peer_length
 * @param data_length
 */
#define teoLNullBufferSize(peer_length, data_length) \
    ( sizeof(teoLNullCPacket) + peer_length + data_length )


/**
 * Initialize L0 client library.
 * 
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
 * Calls once per application to cleanup this client library.
 */
void teoLNullCleanup() {
    
    // Cleanup socket library
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
    
    pkg->cmd = command;
    pkg->data_length = data_length;
    pkg->peer_name_length = peer_name_length;    
    memcpy(pkg->peer_name, peer, pkg->peer_name_length);
    memcpy(pkg->peer_name + pkg->peer_name_length, data, pkg->data_length);
    pkg->checksum = teoByteChecksum(pkg->peer_name, pkg->peer_name_length + 
            pkg->data_length);
    pkg->header_checksum = teoByteChecksum(pkg, sizeof(teoLNullCPacket) - 
            sizeof(pkg->header_checksum));
    
    return sizeof(teoLNullCPacket) + pkg->peer_name_length + pkg->data_length;
}

/**
 * Send packet to L0 server
 * 
 * @param fd L0 server socket
 * @param pkg Package to send
 * @param pkg_length Package length
 * 
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullPacketSend(int fd, void* pkg, size_t pkg_length) {
    
    int snd;
    
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    if((snd = send(fd, pkg, pkg_length, 0)) >= 0);                
    #else
    if((snd = write(fd, pkg, pkg_length)) >= 0);                
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
    if((snd = teoLNullPacketSend(con->fd, buf, pkg_length)) >= 0);
    
    free(buf);
    
    return snd;
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
ssize_t teoLNullPacketSplit(teoLNullConnectData *kld, void* data, 
        size_t data_len, ssize_t received) {
    
    size_t retval = -1;
    
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
    size_t len;

    // \todo Check packet

    // Process read buffer
    if(kld->read_buffer_ptr - kld->last_packet_ptr > sizeof(teoLNullCPacket) && 
       kld->read_buffer_ptr - kld->last_packet_ptr >= 
            (len = sizeof(teoLNullCPacket) + packet->peer_name_length + 
            packet->data_length)) {

        // Check checksum
        uint8_t header_checksum = teoByteChecksum(packet, 
                sizeof(teoLNullCPacket) - 
                sizeof(packet->header_checksum));
        uint8_t checksum = teoByteChecksum(packet->peer_name, 
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
 * @param fd L0 server socket
 * @param pkg Buffer to receive
 * @param pkg_length Buffer length
 * 
 * @return Length of send data
 */
ssize_t teoLNullPacketRecv(int fd, void* buf, size_t buf_length) {
    
    int rc;
    
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    rc = recv(fd, buf, buf_length, 0);
    #else
    rc = read(fd, buf, buf_length);
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
 * @retval -1 Packet not receiving yet (got part of packet)
 * @retval -2 Wrong packet received (dropped)
 */
ssize_t teoLNullRecv(teoLNullConnectData *con) {
    
    char buf[L0_BUFFER_SIZE];
    
    ssize_t rc = teoLNullPacketRecv(con->fd, buf, L0_BUFFER_SIZE);
    rc = teoLNullPacketSplit(con, buf, L0_BUFFER_SIZE, rc != -1 ? rc : 0);
    
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
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param host_name Name of this L0 client
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
    if((snd = teoLNullPacketSend(con->fd, buf, pkg_length)) >= 0);    
    
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
uint8_t teoByteChecksum(void *data, size_t data_length) {
    
    int i;
    uint8_t *ch, checksum = 0;
    for(i = 0; i < (int)data_length; i++) {
        
        ch = (uint8_t*)((char*)data + i);
        checksum += *ch;
    }
    
    return checksum;
}

/**
 * Set socket or FD to non blocking mode
 * 
 * @param fd
 */
void set_nonblock(int fd) {

    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    int iResult;
    u_long iMode = 1;

    iResult = ioctlsocket(fd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
      printf("ioctlsocket failed with error: %ld\n", iResult);
    
    #else
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    #endif
}

/**
 * Set TCP NODELAY option
 * 
 * @param fd TCP socket descriptor
 * 
 * @return Result of setting. Success if >= 0.
 */
int set_tcp_nodelay(int fd) {

    int result = 0;    
    int flag = 1;
    
    result = setsockopt(fd,           // socket affected
                         IPPROTO_TCP,     // set option at TCP level
                         TCP_NODELAY,     // name of option
                         (char *) &flag,  // the cast is historical cruft
                         sizeof(flag));   // length of option value
    if (result < 0) {
        
        printf("Set TCP_NODELAY of fd %d error\n", fd);
    }

    return result;
}

/**
 * Create TCP client and connect to server
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
teoLNullConnectData* teoLNullConnect(int port, const char *server) {

    // Variable and structure definitions.
    int rc;
    struct hostent *hostp;
    struct sockaddr_in serveraddr;
    teoLNullConnectData *con = malloc(sizeof(teoLNullConnectData));
    con->last_packet_ptr = 0;
    con->read_buffer = NULL;
    con->read_buffer_ptr = 0;
    con->read_buffer_size = 0;

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
        return con;
    }
    else {
        #ifdef DEBUG_MSG
        printf("Client-socket() OK\n");
        #endif
    }

    printf("Connecting to the server %s at port %d ...\n", server, port);

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
            return con;
        }
        memcpy(&serveraddr.sin_addr, hostp->h_addr, 
                sizeof(serveraddr.sin_addr));
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
        return con;
    }
    else {
        printf("Connection established ...\n");
    }

    // Set non block mode
    set_nonblock(con->fd);
    
    // Set TCP_NODELAY option
    set_tcp_nodelay(con->fd);

    return con;
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
    }
}

#undef DEBUG_MSG
