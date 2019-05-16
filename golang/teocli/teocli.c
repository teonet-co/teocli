#include <stdio.h>

#include "utils.h"
#include "teonet_l0_client.h"

void _AGoEventCb(void *con, teoLNullEvents event, void *data, size_t data_len, void *user_data);

typedef void (*fptr)(void *con, teoLNullEvents event, void *data, size_t data_len, void *user_data);

fptr getAGoEventCb();

teoLNullConnectData* AC_teoLNullConnectE(const char *server, int port,
	void *user_data) {
	return teoLNullConnectE(server, port, getAGoEventCb(), user_data);
}

void AC_event_cb(void *con, teoLNullEvents event, void *data,
            size_t data_len, void *user_data) {
		getAGoEventCb()(con, event, data, data_len, user_data);
}

const char * AC_peer_name(teoLNullCPacket *cp) {
	return cp->peer_name;
}

const char * AC_get_cmd_data(teoLNullCPacket *cp) {
	return cp->peer_name + cp->peer_name_length;
}

/**
 * Format peers table
 * 
 * @param  cp Pointer to teoLNullCPacket
 * @return Cstring with peers table (should be free after use)
 */ 
const char * AC_show_peers(teoLNullCPacket *cp) {

    char *retval = NULL;

    // Show peer list
    if(cp->data_length > 1) {

        ksnet_arp_data_ar *arp_data_ar = (ksnet_arp_data_ar *)
                (cp->peer_name + cp->peer_name_length);
        const char *ln = "-----------------------------------------------------\n";
        retval = ksnet_formatMessage("%sPeers (%u): \n%s", ln, arp_data_ar->length, ln);
        int i;
        for(i = 0; i < (int)arp_data_ar->length; i++) {

            retval = ksnet_sformatMessage(retval, "%s%-12s(%2d)   %-15s   %d %8.3f ms\n",
                retval,
                arp_data_ar->arp_data[i].name,
                arp_data_ar->arp_data[i].data.mode,
                arp_data_ar->arp_data[i].data.addr,
                arp_data_ar->arp_data[i].data.port,
                arp_data_ar->arp_data[i].data.last_triptime);
        }
        retval = ksnet_sformatMessage(retval, "%s%s", retval, ln);
    }

    return retval;
}
