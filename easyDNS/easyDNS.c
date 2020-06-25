#include <stdlib.h>
#include <stdio.h>
#include "easyDNS.h"
#include "dns_socket.h"
easyDNS_t* easyDNS_init(dns_cb_t cb, int port)
{
	easyDNS_t* server = (easyDNS_t*)calloc(1, sizeof(easyDNS_t));
	server->ds = dns_socket_init(port);
	assert(server->ds);
	server->callback = cb;
	assert(cb);
	return server;
}

void easyDNS_run(easyDNS_t* server)
{
	while(1){
#define SIZE 1024
		struct sockaddr_in addr;
		uint8_t buf[SIZE];
		int len = dns_socket_recv(server->ds, buf, SIZE, &addr);
		if(len < 3){ continue; }
		uint16_t id = (buf[0] << 8) + buf[1];
		struct sockaddr_in *rec_addr = &server->rec_addr[id];
		if(buf[2] & 0x80){
			dns_socket_send(server->ds, buf, len, rec_addr);
			continue;
		}

		uint8_t temp1[] = { 0x00, 0x01, 0x00, 0x00,  0x00, 0x00,  0x00, 0x00, };
		uint8_t temp2[] = { 0x00, 0x01, 0x00, 0x01};
		if(!memcmp(&buf[4], temp1, sizeof(temp1)) &&
				!memcmp(&buf[len-4], temp2, sizeof(temp2))){
			char domain[SIZE], *pd = domain;
			memcpy(domain, &buf[12], len-12-4);
			while(*pd && pd-domain < len){
				uint8_t len = *pd;
				*pd = '.';
				pd += len+1;
			}
			uint32_t ip;
			if((ip = server->callback(&domain[1]))!=0){
				uint8_t reply[len+len-12+10];
				memcpy(reply, buf, len);
				reply[2] = 0x80;
				reply[7] = 0x01;
				memcpy(&reply[len], &buf[12], len-12);
				uint8_t temp[] = {
					0x00, 0x00, 0x00, 0x40, 0x00, 0x04,
				};
				memcpy(&reply[sizeof(reply)-10], temp, sizeof(temp));
				reply[sizeof(reply)-4] = (ip >> 24) & 0xFF;
				reply[sizeof(reply)-3] = (ip >> 16) & 0xFF;
				reply[sizeof(reply)-2] = (ip >>  8) & 0xFF;
				reply[sizeof(reply)-1] = (ip >>  0) & 0xFF;
				dns_socket_send(server->ds, reply, sizeof(reply), &addr);
				continue;
			}
		}
		*rec_addr = addr;
		struct sockaddr_in myaddr;
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(53);
		myaddr.sin_addr.s_addr = DNS_SERVER;
		dns_socket_send(server->ds, buf, len, &myaddr);
	}

}

#include <pthread.h>
static void easyClientDNS_run(easyClientDNS_t* client);
easyClientDNS_t* easyClientDNS_init(ip_cb_t cb, int port)
{
	easyClientDNS_t *client = (easyClientDNS_t*)calloc(1, sizeof(easyClientDNS_t));
	client->ds = dns_socket_init(port);
	assert(client->ds);
	client->callback = cb;
	assert(cb);
	easyClientDNS_run(client);
	return client;
}

static void* easyClientDNS_th(void*p)
{
	easyClientDNS_t* client = (easyClientDNS_t*)p;
	while(1){
#define SIZE 1024
		struct sockaddr_in addr;
		uint8_t buf[SIZE];
		int len = dns_socket_recv(client->ds, buf, SIZE, &addr);
		if(len < 3){ continue; }

		uint8_t *domain= &buf[13], *cur = &buf[12];
		while(*cur && cur-buf<len){
			char *p = cur;
			cur += *p + 1;
			*p = '.';
		}
		for(uint8_t *a = cur + 5;a-buf<len;){
			uint16_t res_type = (a[2]<<8)+a[3];
			uint16_t res_class = (a[4]<<8)+a[5];
			uint16_t res_len = (a[10]<<8)+a[11];
			if(res_type == 1 && res_class == 1 && res_len == 4){
				uint32_t ip = (a[12] << 0) | (a[13] << 8) | (a[14] << 16) | (a[15] << 24);
				client->callback(domain, ip, addr.sin_addr.s_addr);
			}
			a += 12 + res_len;
		}
	}
}
static void easyClientDNS_run(easyClientDNS_t* client)
{
	pthread_t th;
	pthread_create(&th, NULL, easyClientDNS_th, client);
}
void easyQueryDNS(easyClientDNS_t* client, char *domain, uint32_t dnsServer)
{
	uint8_t query[1024] = {
		0x4b, 0x65, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x03, 0x77, 0x77, 0x77,
		0x05, 0x62, 0x61, 0x69, 0x64, 0x75, 0x03, 0x63,
		0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01
	};
	query[0] = rand(); query[1] = rand();
	uint8_t *domain_s = &query[12], *domain_e;
	domain_e = domain_s + strlen(domain) + 2;
	strcpy((char*)domain_s+1, domain);
	domain_e[0] = 0x00; domain_e[1] = 0x01;
	domain_e[2] = 0x00; domain_e[3] = 0x01;

	for(uint8_t *p=domain_e-2, len=0;p>=domain_s;p--){
		if(*p == '.'){
			*p = len;
			len = 0;
		}else{
			len++;
		}
	}
	int total_len = domain_e - query + 4;
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(53);
	myaddr.sin_addr.s_addr = dnsServer;
	dns_socket_send(client->ds, query, total_len, &myaddr);
}
