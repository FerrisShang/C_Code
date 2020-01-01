#include "squeue.h"
#include "win_serial.h"
#include "script_parse.h"
#include "stdio.h"
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex mtx;

#define debug(...) printf(__VA_ARGS__)

void create_parse(const char *com, int baud, const char *filename)
{
	CBtIO btio;
	int res = btio.start(com, baud);
	if(res != SERIAL_SUCCESS){
		debug("Open uart \"%s\" failed: %d\n", com, res);
		return;
	}
	CScriptParse scParse((char*)filename, com, &mtx);

#define RECV_MAX_LEN 1024
	uint8_t *received = NULL, recv_buf[RECV_MAX_LEN];
	int recv_len = 0;
	vector<vector<uint8_t>> send_data;
	while(1){
		send_data = scParse.get_send_data(received, recv_len);
		if(scParse.isFinished()){ break; }
		FOR(i, send_data.size()){
			btio.send(send_data[i]);
			if(!scParse.getDebug())continue;
			mtx.lock();
			debug(com);
			debug("-<<: "); FOR(j, send_data[i].size()){ debug("%02X ", send_data[i][j]); } debug("\n");
			mtx.unlock();
		}
		recv_len = btio.recv(recv_buf, RECV_MAX_LEN, scParse.get_timeout());
		if(recv_len < 0){ mtx.lock(); debug(com); puts(": Timeout !"); break; mtx.unlock(); }
		if(scParse.getDebug()){
			mtx.lock();
			debug(com);
			debug("->>: "); FOR(j, recv_len){debug("%02X ", recv_buf[j]); } debug("\n");
			mtx.unlock();
		}
		received = recv_buf;
	}
	btio.stop();
}

int main(int argc, char *argv[])
{
	if(argc < 2){
		vector<int>ports = CSerial::scan_port();
		debug("Usage: command COM_NUM baudrate[ ");
		for(int i=0;i<ports.size();i++){
			debug("COM%d ", ports[i]);
		}
		debug("] filename ...\n");
		exit(0);
	}
	vector<thread> threads;
	for(int i=0;i<(argc-1)/3;i++){
		threads.push_back(thread(create_parse, argv[i*3+1], atoi(argv[i*3+2]), argv[i*3+3]));
	}
	for(auto& th : threads) if(th.joinable())th.join();
	return 0;
}

