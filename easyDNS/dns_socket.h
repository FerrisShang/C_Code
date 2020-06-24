#ifndef __DNS_SOCKET_H__
#define __DNS_SOCKET_H__

#include <stdint.h>
#include <assert.h>
#if __linux__ 
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <windows.h>
#endif

#define DNS_SERVER_PORT 5353
typedef struct{
	int fd;
    struct sockaddr_in address;
} dns_socket_t;

dns_socket_t* dns_socket_init(void);

int dns_socket_recv(dns_socket_t *ds, uint8_t *buf,
		uint32_t len, struct sockaddr_in *addr);

int dns_socket_send(dns_socket_t *ds, uint8_t *buf,
		uint32_t len, struct sockaddr_in *addr);

#endif
