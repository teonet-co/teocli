package teocli

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

#cgo CFLAGS:  -I../../libteol0 -std=c11
#cgo LDFLAGS: libteocli.a

*/
import "C"

import (
	"unsafe"
)

type agoEventCb func(con *C.teoLNullConnectData, event C.teoLNullEvents,
	data unsafe.Pointer, dataLen C.size_t, userData unsafe.Pointer)

// AGoEventCb teocli callback
// export _AGoEventCb
var _AGoEventCb agoEventCb

//export getAGoEventCb
func getAGoEventCb() agoEventCb {
	return _AGoEventCb
}

// LNullConnectE Connect to L0 server and create connection
func LNullConnectE(tcpServer string, tcpPort int, cbFunc agoEventCb, param unsafe.Pointer) *C.teoLNullConnectData {
	_AGoEventCb = cbFunc
	_tcpServer := C.CString(tcpServer)
	defer C.free(unsafe.Pointer(_tcpServer))
	return C.AC_teoLNullConnectE(_tcpServer, C.int(tcpPort), param)
}

// GetCmdData return command data
func GetCmdData(cp *C.teoLNullCPacket) unsafe.Pointer {
	return unsafe.Pointer(C.AC_get_cmd_data(cp))
}

// ShowPeers return Format peers table
func ShowPeers(cp *C.teoLNullCPacket) string {
	peers := C.AC_show_peers(cp)
	defer C.free(unsafe.Pointer(peers))
	return C.GoString(peers)
}

// PeerName return peer name
func PeerName(cp *C.teoLNullCPacket) string {
	return C.GoString(C.AC_peer_name(cp))
}

//export C.teoLNullCPacket

// CPacket return pointer to teoLNullCPacket
func CPacket(data unsafe.Pointer) *C.teoLNullCPacket {
	return (*C.teoLNullCPacket)(data)
}
