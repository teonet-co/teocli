// teocli
package main

/*
#include "teonet_l0_client.h"

extern void AGoEventCb(void *con, teoLNullEvents event, void *data,
            size_t data_len, void *user_data);

teoLNullConnectData* AC_teoLNullConnectE(const char *server, int port,
	void *user_data) {
	return teoLNullConnectE(server, port, AGoEventCb, user_data);
}

void AC_event_cb(void *con, teoLNullEvents event, void *data,
            size_t data_len, void *user_data) {
		AGoEventCb(con, event, data, data_len, user_data);
}

const char * AC_peer_name(teoLNullCPacket *cp) {
	return cp->peer_name;
}

const char * AC_get_cmd_data(teoLNullCPacket *cp) {
	return cp->peer_name + cp->peer_name_length;
}

*/
import "C"
