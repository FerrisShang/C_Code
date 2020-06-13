#include "dns_socket.h"

dns_socket_t* dns_socket_init(void)
{
#if !__linux__ 
    WSADATA data;
    assert(!WSAStartup(MAKEWORD(2,2), &data) != 0);
#endif
	dns_socket_t *ds = (dns_socket_t*)malloc(sizeof(dns_socket_t));
	ds->fd = socket(AF_INET, SOCK_DGRAM, 0);
    ds->address.sin_family = AF_INET;
    ds->address.sin_addr.s_addr = INADDR_ANY;
    ds->address.sin_port = htons(DNS_SERVER_PORT);
    assert(!bind(ds->fd, (struct sockaddr *)&ds->address,
				sizeof (struct sockaddr_in)));
	return ds;
}

int dns_socket_send(dns_socket_t *ds, uint8_t *buf,
		uint32_t len, struct sockaddr_in *addr)
{
    int addrLen = sizeof (struct sockaddr_in);
	return sendto(ds->fd, buf, len, 0, (struct sockaddr*)addr, addrLen);
}

int dns_socket_recv(dns_socket_t *ds, uint8_t *buf,
		uint32_t len, struct sockaddr_in *addr)
{
    int addrLen = sizeof (struct sockaddr_in);
    return recvfrom(ds->fd, buf, len, 0, (struct sockaddr*)addr, &addrLen);
}


