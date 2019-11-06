#include "win_serial.h"
#include "stdio.h"
#include <iostream>

using namespace std;
static vector<uint8_t> s2b(const string& stream);
#define DUMP(d) do{for(int i=0;i<d.size();i++){printf("%02X ",d[i]);}printf("\n");}while(0)

void cb(vector<uint8_t>& data, void *p){
#define SEND(s) do{((CBtIO*)p)->send(s2b(s));}while(0)
#define IF(s) else if(data==s2b(s))
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

static vector<uint8_t> s2b(const string& stream){
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


