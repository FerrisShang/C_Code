#ifndef __EASY_DNS_H__
#define __EASY_DNS_H__

#include "dns_socket.h"

#define DNS_SERVER 0x08080808

typedef uint32_t (*dns_cb_t)(char* domain);
typedef struct {
	dns_socket_t *ds;
	dns_cb_t callback;
	struct sockaddr_in rec_addr[65536];
}easyDNS_t;

easyDNS_t* easyDNS_init(dns_cb_t cb);
void easyDNS_run(easyDNS_t* server);

#endif
