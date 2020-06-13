#include "easyDNS.h"
#include "dns_socket.h"
easyDNS_t* easyDNS_init(dns_cb_t cb)
{
	easyDNS_t* server = (easyDNS_t*)calloc(1, sizeof(easyDNS_t));
	server->ds = dns_socket_init();
	assert(server->ds);
	server->callback = cb;
	assert(cb);
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


