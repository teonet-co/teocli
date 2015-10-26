/** 
 * \file   teonet_l0_client.h
 * \author Kirill Scherba <kirill@scherba.ru>
 * 
 * Created on October 12, 2015, 12:32 PM
 */

/**
 * @mainpage Teocli library Documentation  
 * 
 * * [Native library structures and functions](teonet__l0__client_8h.html) 
 * * [C++ teocli class wrapper](classteocli.html)  
 * * [Basic example](main_8c-example.html)    
 */

#ifndef TEONET_L0_CLIENT_H
#define	TEONET_L0_CLIENT_H

#include <stdint.h>
#ifdef HAVE_MINGW
#include <winsock2.h>
#endif

/**
 * L0 System commands
 */
enum CMD_L {
    
    CMD_L_ECHO = 65,    ///< Echo command
    CMD_L_ECHO_ANSWER,  ///< Answer to echo command
    CMD_L_PEERS = 72,   ///< Get peers command
    CMD_L_PEERS_ANSWER  ///< Answer to get peers command
            
};

#define L0_BUFFER_SIZE 2048

/**
 * L0 client connect data
 */
typedef struct teoLNullConnectData {

    #ifndef HAVE_MINGW
    int fd;             ///< Connection socket
    #else
    SOCKET fd;          ///< Connection socket
    #endif
    
    void *read_buffer;      ///< Pointer to saved buffer
    size_t read_buffer_ptr; ///< Pointer in read buffer
    size_t read_buffer_size;///< Read buffer size
    size_t last_packet_ptr; ///< Last recived packet pointer (length)

} teoLNullConnectData;
        
/**
 * L0 client packet data structure
 * 
 */        
typedef struct teoLNullCPacket {

    uint8_t cmd; ///< Command
    uint8_t peer_name_length; ///< To peer name length (include leading zero)
    uint16_t data_length; ///< Packet data length
    uint8_t reserved_1; ///< Reserved 1
    uint8_t reserved_2; ///< Reserved 2
    uint8_t checksum; ///< Whole checksum
    uint8_t header_checksum; ///< Header checksum
    char peer_name[]; ///< To/From peer name (include leading zero) + packet data

} teoLNullCPacket;


#ifdef	__cplusplus
extern "C" {
#endif

void teoLNullInit();  
void teoLNullCleanup();

teoLNullConnectData *teoLNullClientConnect(int port, const char *server);
void teoLNullClientDisconnect(teoLNullConnectData *con);
size_t teoLNullClientLogin(void* buffer, size_t buffer_length, const char* host_name);

size_t teoLNullPacketCreate(void* buffer, size_t buffer_length, uint8_t command, 
        const char * peer, const void* data, size_t data_length);
ssize_t teoLNullPacketSend(int fd, void* pkg, size_t pkg_length);
ssize_t teoLNullPacketRecv(int fd, void* buf, size_t buf_length);
ssize_t teoLNullPacketSplit(teoLNullConnectData *kld, void* data, 
        size_t data_len, ssize_t received);
ssize_t teoLNullPacketRecvS(teoLNullConnectData *con);

uint8_t teoByteChecksum(void *data, size_t data_length);

#ifdef	__cplusplus
}
#endif

#endif	/* TEONET_L0_CLIENT_H */
