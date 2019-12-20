#include "squeue.h"
#include "win_serial.h"
#include "script_parse.h"
#include "stdio.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	CBtIO btio;
	btio.start(4, 115200);
	CScriptParse scParse((char*)"demo.sc");

#define RECV_MAX_LEN 1024
	uint8_t *received = NULL, recv_buf[RECV_MAX_LEN];
	static int recv_len = 0;
	vector<vector<uint8_t>> send_data;
	while(1){
		send_data = scParse.get_send_data(received, recv_len);
		FOR(i, send_data.size()){
			btio.send(send_data[i]);
		}
		recv_len = btio.recv(recv_buf, RECV_MAX_LEN, scParse.get_timeout());
		if(recv_len < 0){ puts("Timeout !"); exit(0); }
		received = recv_buf;
	}
}

