#include "winsock2.h"
#include "win_serial.h"
#include "stdio.h"
#include <iostream>

using namespace std;
mutex mtx;
vector<uint8_t> s2b(const string& stream);
void udp_send(const vector<uint8_t>& data);
#define DUMP(d) do{mtx.lock();for(int i=0;i<d.size();i++){printf("%02X ",d[i]);}printf("\n");udp_send(d);mtx.unlock();}while(0)
#define XXXX 0xFE
bool same(const vector<uint8_t>& d1, const vector<uint8_t>& d2){
	if(d2.size()>d1.size()) return false;
	for(int i=0;i<min(d1.size(), d2.size());i++){if(d1[i]!=d2[i]&&d2[i]!=XXXX)return false;}
	return true;
}
#define SEND(s) do{((CBtIO*)p)->send(s2b(s));}while(0)
#define SENDB(b) do{((CBtIO*)p)->send(b);}while(0)
#define IF(s) else if(same(data, s2b(s)))

void cb(vector<uint8_t>& data, void *p){
	DUMP(data);
	if(0){
	}IF("040e0405030c00"){
		SEND("05020d0d001000010001");
	}
}
int main(int argc, char *argv[])
{
	if(argc < 3){printf("Usage: Command Port Baudrate\n"); return 0;}
	CBtIO btio;
	btio.setCallback(cb, &btio);
	int err = btio.start((char*)argv[1], atoi(argv[1]));
	if(err){ cout << "Open port error\n"; return -1; }
	btio.send((uint8_t*)"\x01\x03\x0c\x00", 4);
	while(1)Sleep(~0);
}

vector<uint8_t> s2b(const string& stream){
	vector<uint8_t> hex;
	for(int i=0;i<stream.length();i++){
		if('0'<=stream[i]&&stream[i]<='9'){
			hex.push_back((uint8_t)stream[i]);
		}else if(('a'<=stream[i]&&stream[i]<='f')||('A'<=stream[i]&&stream[i]<='F')){
			hex.push_back((uint8_t)stream[i] | 0x20);
		}else if(isspace(stream[i])){
			continue;
		}else{ break; }
	}
	for(int i=0;i<hex.size()/2;i++){
		uint8_t byte = hex[i+i+0] >= 'a'?(10+hex[i+i+0]-'a'):(hex[i+i+0]-'0');
		hex[i] = (byte << 4) | (hex[i+i+1] >= 'a'?(10+hex[i+i+1]-'a'):(hex[i+i+1]-'0'));
	}
	hex.resize(hex.size()/2);
	return hex;
}

void udp_send(const vector<uint8_t>& data)
{
	#define MAX_RECV_LEN 2048
	static char IP[32] = "192.168.5.10";
	static int PORT_REMOTE = 44544;
	static SOCKET sockfd;
	static struct sockaddr_in sin;
	static int len_sin = sizeof(sin);
	if(sockfd == 0){
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		sin.sin_family = AF_INET;
		sin.sin_port = htons(PORT_REMOTE);
		sin.sin_addr.S_un.S_addr = inet_addr(IP);
	}
	sendto(sockfd, (const char*)&data[0], data.size(), 0, (struct sockaddr*)&sin, len_sin);
}



