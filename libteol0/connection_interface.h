/**
 * \file   connection_interface.h
 * \author max <mpano91@gmail.com>
 */

#ifndef CONNECTION_INTERFACE_H
#define	CONNECTION_INTERFACE_H


typedef struct connection_interface_t {

  ssize_t (*send_echo)(struct connection_interface_t*, const char *peer_name, const char *msg);
  int (*get_connection_status)(struct connection_interface_t*);
  void *impl_;

} connection_interface_t;


#endif	/* CONNECTION_INTERFACE_H */
