#ifndef __EASY_DNS_H__
#define __EASY_DNS_H__

#include "dns_socket.h"

#define DNS_SERVER 0x08080808
#define DNS_SERVER_PORT 53

typedef uint32_t (*dns_cb_t)(char* domain);
typedef struct {
	dns_socket_t *ds;
	dns_cb_t callback;
	struct sockaddr_in rec_addr[65536];
}easyDNS_t;

easyDNS_t* easyDNS_init(dns_cb_t cb, int port);
void easyDNS_run(easyDNS_t* server);


typedef void (*ip_cb_t)(char* domain, uint32_t ip, uint32_t dns_ip);
typedef struct {
	dns_socket_t *ds;
	ip_cb_t callback;
}easyClientDNS_t;
easyClientDNS_t* easyClientDNS_init(ip_cb_t cb, int port);
void easyQueryDNS(easyClientDNS_t* client, char *domain, uint32_t dnsServer);

#endif
