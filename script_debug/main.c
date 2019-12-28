#include "squeue.h"
#include "win_serial.h"
#include "script_parse.h"
#include "stdio.h"
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex mtx;

#define debug(...) do{mtx.lock(); printf(__VA_ARGS__); mtx.unlock();}while(0)

void create_parse(const char *com, int baud, const char *filename)
{
	CBtIO btio;
	int res = btio.start(com, baud);
	if(res != SERIAL_SUCCESS){
		printf("Open uart \"%s\" failed: %d\n", com, res);
		return;
	}
	CScriptParse scParse((char*)filename, &mtx);

#define RECV_MAX_LEN 1024
	uint8_t *received = NULL, recv_buf[RECV_MAX_LEN];
	int recv_len = 0;
	vector<vector<uint8_t>> send_data;
	while(1){
		send_data = scParse.get_send_data(received, recv_len);
		FOR(i, send_data.size()){
			btio.send(send_data[i]);
			mtx.lock();
			printf(com);
			printf("-<<: "); FOR(j, send_data[i].size()){ printf("%02X ", send_data[i][j]); } printf("\n");
			mtx.unlock();
		}
		recv_len = btio.recv(recv_buf, RECV_MAX_LEN, scParse.get_timeout());
		if(recv_len < 0){ puts("Timeout !"); return; }
		mtx.lock();
		printf(com);
		printf("->>: "); FOR(j, recv_len){printf("%02X ", recv_buf[j]); } printf("\n");
		mtx.unlock();
		received = recv_buf;
	}
}

int main(int argc, char *argv[])
{
	if(argc < 2){
		vector<int>ports = CSerial::scan_port();
		printf("Usage: command COM_NUM baudrate[ ");
		for(int i=0;i<ports.size();i++){
			printf("COM%d ", ports[i]);
		}
		printf("] filename ...\n");
		exit(0);
	}
	vector<thread> threads;
	for(int i=0;i<(argc-1)/3;i++){
		threads.push_back(thread(create_parse, argv[i*3+1], atoi(argv[i*3+2]), argv[i*3+3]));
	}
	for(auto& th : threads) if(th.joinable())th.join();
	return 0;
}

