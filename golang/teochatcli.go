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
type app_parameters struct {
	host_name  string
	tcp_server string
	tcp_port   int
	peer_name  string
	msg        string
}

// Teonet event loop callback
//export AGo_event_cb
func AGo_event_cb(con *C.teoLNullConnectData, event C.teoLNullEvents,
	data unsafe.Pointer, data_len C.size_t, user_data unsafe.Pointer) {

	param := (*app_parameters)(user_data)

	switch event {

	// Connected to teonet
	case C.EV_L_CONNECTED:
		fd := (*C.int)(data)
		if *fd > 0 {
			host_name := C.CString(param.host_name)
			defer C.free(unsafe.Pointer(host_name))
			fmt.Println("Successfully connect to server")
			// Send (1) Initialization packet to L0 server
			C.teoLNullLogin(con, host_name)
		}

	// Disconnected from teonet
	case C.EV_L_DISCONNECTED:
		fmt.Println("Disconnected ...")

	// Receive answer from server
	case C.EV_L_RECEIVED:
		rc := data_len
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
			trip_time := C.teoLNullProccessEchoAnswer(data)
			fmt.Printf("Data: %s\n", C.GoString(data))    // Show data
			fmt.Printf("Trip time: %d ms\n\n", trip_time) // Show trip time

		case C.CMD_L_AUTH_LOGIN_ANSWER:
			fmt.Println("Got answer from authentication server")
			auth_data := C.AC_get_cmd_data(cp)
			fmt.Printf("Data: %s\n\n", C.GoString(auth_data)) // Show data

		case C.CMD_L_ECHO:
			fmt.Println("Got echo command")

		default:
			fmt.Println("Got unknown command")
		}
	}
}

// Parce application command line parameters
func parse_params() (*app_parameters, bool) {
	param := new(app_parameters)

	// Check application parameters
	if len(os.Args) < 5 {
		fmt.Printf("Usage: %s <client_name> <server_address> <server_port> "+
			"<peer_name> [message]\n", os.Args[0])
		return param, true
	}

	// Teonet L0 server parameters
	param.host_name = os.Args[1]       // Host name
	param.tcp_server = os.Args[2]      // Server name or ip
	i, err := strconv.Atoi(os.Args[3]) // Server port
	if err != nil {
		param.tcp_port = 9000
	} else {
		param.tcp_port = i
	}
	param.peer_name = os.Args[4] // Peer name to send ping to
	if len(os.Args) > 5 {
		param.msg = os.Args[5]
	} else {
		param.msg = "Hello"
	}
	return param, false
}

// Start teonet
func start_teonet(param *app_parameters) {
	// Initialize L0 Client library
	C.teoLNullInit()

	// Connect to L0 server
	tcp_server := C.CString(param.tcp_server)
	defer C.free(unsafe.Pointer(tcp_server))
	con := C.AC_teoLNullConnectE(tcp_server, C.int(param.tcp_port),
		unsafe.Pointer(param))
	if con.fd > 0 {
		num := 0
		timeout := 50
		msg := C.CString(param.msg)
		peer_name := C.CString(param.peer_name)
		defer C.free(unsafe.Pointer(msg))
		defer C.free(unsafe.Pointer(peer_name))

		// Event loop
		for C.teoLNullReadEventLoop(con, C.int(timeout)) != 0 {

			// Send Echo command every second
			if (num % (1000 / timeout)) == 0 {
				fmt.Println("Send ping...")
				C.teoLNullSendEcho(con, peer_name, msg)
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
	fmt.Println("Teonet chat Go client version 0.0.1 (Native TCP Client)\n")

	// Parse application parameters
	param, err := parse_params()
	if err {
		return
	}

	// Start teonet
	start_teonet(param)
}
