#include "squeue.h"
#include "win_serial.h"
#include "script_parse.h"
#include "stdio.h"
#include "windows.h"
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
		FOR(i, send_data.size()){
			if(send_data[i][0] == SC_CMD_DELAY){
				uint32_t delay_ms = (send_data[i][1])+(send_data[i][2]<<8)+
					(send_data[i][3]<<16)+(send_data[i][4]<<24);
				if(scParse.getDebug()){
					//mtx.lock(); debug(com); debug(": delay %d ms\n", delay_ms); mtx.unlock();
				}
				Sleep(delay_ms);
			}else if(send_data[i][0] == SC_CMD_SEND){
				btio.send(&send_data[i][1], send_data[i].size()-1);
				if(scParse.getDebug() <= SC_CMD_DEBUG-SC_CMD_INFO){
					mtx.lock();
					debug(com);
					debug("-<<: "); for(int j=1;j<send_data[i].size();j++){
						debug("%02X ", send_data[i][j]); } debug("\n");
					mtx.unlock();
				}
			}else if(send_data[i][0] == SC_CMD_RECV){
				recv_len = btio.recv(recv_buf, RECV_MAX_LEN, scParse.get_timeout());
				if(recv_len < 0){
					mtx.lock(); debug(com); puts(": Timeout !"); mtx.unlock();
					btio.stop(); return;
				}
				if(scParse.getDebug() <= SC_CMD_DEBUG-SC_CMD_INFO){
					mtx.lock();
					debug(com);
					debug("->>: "); FOR(j, recv_len){debug("%02X ", recv_buf[j]); } debug("\n");
					mtx.unlock();
				}
				received = recv_buf;
			}else if((send_data[i][0] == SC_CMD_INFO && scParse.getDebug()<=SC_CMD_INFO-SC_CMD_INFO) ||
					(send_data[i][0] == SC_CMD_DEBUG && scParse.getDebug()<=SC_CMD_DEBUG-SC_CMD_INFO) ||
					(send_data[i][0] == SC_CMD_WARNING && scParse.getDebug()<=SC_CMD_WARNING-SC_CMD_INFO) ||
					(send_data[i][0] == SC_CMD_ERROR && scParse.getDebug()<=SC_CMD_ERROR-SC_CMD_INFO)){
				mtx.lock(); debug(com); debug(": %s\n", &send_data[i][1]); mtx.unlock();
			}else if(send_data[i][0] == SC_CMD_CUSTOM){
				if(!memcmp(&send_data[i][1], "FLUSH", min((int)(sizeof("FLUSH")-1), (int)send_data[i][0]))){
					btio.clear();
				}
			}
		}
		if(scParse.isFinished()){
			mtx.lock();
			debug(com);
			debug(": script end.\n");
			mtx.unlock();
			break;
		}
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

