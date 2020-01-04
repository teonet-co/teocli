package main

// Commands to build teonet client static library:
//	 gcc -c libteol0/teonet_l0_client.c
//	 ar rcs libteocli.a teonet_l0_client.o
//
// Command to build application:
//	go build -o teochatcli

/*
#include <stdlib.h>
#include "teonet_l0_client.h"

void AC_event_cb(void *con, teoLNullEvents event, void *data,
            size_t data_len, void *user_data);

teoLNullConnectData* AC_teoLNullConnectE(const char *server, int port,
	void *user_data);

const char * AC_peer_name(teoLNullCPacket *cp);

const char * AC_get_cmd_data(teoLNullCPacket *cp);

const char * AC_show_peers(teoLNullCPacket *cp);

#cgo CFLAGS:  -I../libteol0 -std=c11
#cgo LDFLAGS: -L../libteol0 libteocli.a

*/
import "C"

import (
	"fmt"
	"os"
	"strconv"
	"unsafe"
)

// Application parameters structure
type appParameters struct {
	hostName  string
	tcpServer string
	tcpPort   int
	peerName  string
	msg       string
}

//AGoEventCb Teonet event loop callback
//export AGoEventCb
func AGoEventCb(con *C.teoLNullConnectData, event C.teoLNullEvents,
	data unsafe.Pointer, dataLen C.size_t, userData unsafe.Pointer) {

	param := (*appParameters)(userData)

	switch event {

	// Connected to teonet
	case C.EV_L_CONNECTED:
		fd := (*C.int)(data)
		if *fd > 0 {
			hostName := C.CString(param.hostName)
			defer C.free(unsafe.Pointer(hostName))
			fmt.Println("Successfully connect to server")
			// Send (1) Initialization packet to L0 server
			C.teoLNullLogin(con, hostName)
		}

	// Disconnected from teonet
	case C.EV_L_DISCONNECTED:
		fmt.Println("Disconnected ...")

	// Receive answer from server
	case C.EV_L_RECEIVED, C.EV_L_RECEIVED_UNRELIABLE:
		rc := dataLen
		cp := (*C.teoLNullCPacket)(data)

		fmt.Printf("Receive %d bytes: %d bytes data from L0 server, "+
			"from peer %s, cmd = %d\n",
			rc, cp.data_length, C.GoString(C.AC_peer_name(cp)), cp.cmd)

		// Process commands
		switch cp.cmd {

		// Echo answer command
		case C.CMD_L_ECHO_ANSWER:
			fmt.Println("Got echo answer command")
			data := C.AC_get_cmd_data(cp)
			tripTime := C.teoLNullProccessEchoAnswer(data)
			fmt.Printf("Data: %s\n", C.GoString(data))   // Show data
			fmt.Printf("Trip time: %d ms\n\n", tripTime) // Show trip time

		// Auth server ansver
		case C.CMD_L_AUTH_LOGIN_ANSWER:
			fmt.Println("Got answer from authentication server")
			authData := C.AC_get_cmd_data(cp)
			fmt.Printf("Data: %s\n\n", C.GoString(authData)) // Show data
			// Send CMD_L_PEERS command
			snd := C.teoLNullSend(con, C.CMD_L_PEERS, C.CString(param.peerName), nil, 0)
			fmt.Printf("Send %d bytes packet to L0 server to peer %s, cmd = %d (CMD_L_PEERS)\n",
				snd, param.peerName, C.CMD_L_PEERS)

		// Peers command answer
		case C.CMD_L_PEERS_ANSWER:
			fmt.Println("Got answer to CMD_L_PEERS command from peer", C.GoString(C.AC_peer_name(cp)))
			peers := C.AC_show_peers(cp)
			fmt.Println(C.GoString(peers))
			C.free(unsafe.Pointer(peers))

		// Echo answer
		case C.CMD_L_ECHO:
			fmt.Println("Got echo command")

		default:
			fmt.Println("Got unknown command")
		}
	}
}

// Parce application command line parameters
func parseParams() (*appParameters, bool) {
	param := new(appParameters)

	// Check application parameters
	if len(os.Args) < 5 {
		fmt.Printf("Usage: %s <client_name> <server_address> <server_port> "+
			"<peer_name> [message]\n", os.Args[0])
		return param, true
	}

	// Teonet L0 server parameters
	param.hostName = os.Args[1]        // Host name
	param.tcpServer = os.Args[2]       // Server name or ip
	i, err := strconv.Atoi(os.Args[3]) // Server port
	if err != nil {
		param.tcpPort = 9000
	} else {
		param.tcpPort = i
	}
	param.peerName = os.Args[4] // Peer name to send ping to
	if len(os.Args) > 5 {
		param.msg = os.Args[5]
	} else {
		param.msg = "Hello"
	}
	return param, false
}

// Start teonet
func startTeonet(param *appParameters) {
	// Initialize L0 Client library
	C.teoLNullInit()

	// Connect to L0 server
	tcpServer := C.CString(param.tcpServer)
	defer C.free(unsafe.Pointer(tcpServer))
	con := C.AC_teoLNullConnectE(tcpServer, C.int(param.tcpPort),
		unsafe.Pointer(param))
	if con.fd > 0 {
		num := 0
		timeout := 50
		msg := C.CString(param.msg)
		peerName := C.CString(param.peerName)
		defer C.free(unsafe.Pointer(msg))
		defer C.free(unsafe.Pointer(peerName))

		// Event loop
		for C.teoLNullReadEventLoop(con, C.int(timeout)) != 0 {

			// Send Echo command every second
			if (num % (1000 / timeout)) == 0 {
				fmt.Println("Send ping...")
				C.teoLNullSendEcho(con, peerName, msg)
			}
			num++
		}

		// Close connection
		C.teoLNullDisconnect(con)
	}

	// Cleanup L0 Client library
	C.teoLNullCleanup()
}

// Main function
func main() {
	// Welcome message
	fmt.Println("Teonet chat Go client version 0.0.1 (Native TCP Client)" + "\n")

	// Parse application parameters
	param, err := parseParams()
	if err {
		return
	}

	// Start teonet
	startTeonet(param)
}
