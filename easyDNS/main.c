#include <stdio.h>
#include "easyDNS.h"

uint32_t callback(char* domain)
{
	fprintf(stderr, "%s\n", domain);
	fflush(stderr);
	if(strstr(domain, "localhost")) return 0x7F000001;
	return 0;
}

void ip_cb(char* domain, uint32_t ip, uint32_t dns_ip)
{
	fprintf(stderr, "%s ", domain);
	fprintf(stderr, "%d.%d.%d.%d -> ",
			((uint8_t*)&dns_ip)[0], ((uint8_t*)&dns_ip)[1],
			((uint8_t*)&dns_ip)[2], ((uint8_t*)&dns_ip)[3]);
	fprintf(stderr, "%d.%d.%d.%d\n",
			((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1], ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]);
	fflush(stderr);
}

int main(void)
{
	easyClientDNS_t *client = easyClientDNS_init(ip_cb, 6666+(rand()&0x3FF));
	easyQueryDNS(client, "www.google.com", 0x08080808); // 8.8.8.8
	easyQueryDNS(client, "www.google.com", 0xDEDE43D0); // 208.67.222.222
	easyQueryDNS(client, "www.google.com", 0x1A381A08); // 8.26.56.26
	easyQueryDNS(client, "www.google.com", 0x06400640); // 64.6.64.6
	easyQueryDNS(client, "www.google.com", 0x64605667); // 103.86.96.100
	easyQueryDNS(client, "www.google.com", 0x01469A9C); // 156.154.70.1
	easyQueryDNS(client, "www.google.com", 0x01479A9C); // 156.154.71.1
	easyQueryDNS(client, "www.google.com", 0x01020204); // 4.2.2.1
	easyQueryDNS(client, "www.google.com", 0x02020204); // 4.2.2.2

	easyDNS_t *server = easyDNS_init(callback, DNS_SERVER_PORT);
	easyDNS_run(server);
}
