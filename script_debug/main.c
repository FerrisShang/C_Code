#include "squeue.h"
#include "win_serial.h"
#include "script_parse.h"
#include "stdio.h"
#include "delay.h"
#include "mult_sync.h"
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex mtx, var_lock;

#define debug(...) printf(__VA_ARGS__)

vector<CScriptParse*> scParses;
void share_variable(CScriptParse &scParse, char *val_name)
{
	auto val = scParse.get_val(val_name);
	for(CScriptParse* sp : scParses){
		sp->update_variable(val);
	}
}
void create_parse(const char *com, int baud, const char *filename)
{
	CBtIO btio;
	int res = btio.start(com, baud);
	if(res != SERIAL_SUCCESS){
		debug("Open uart \"%s\" failed: %d\n", com, res);
		return;
	}
	CScriptParse scParse((char*)filename, com, &mtx, &var_lock);
	mtx.lock();
	scParses.push_back(&scParse);
	mtx.unlock();

#define RECV_MAX_LEN 1024
	uint8_t *received = NULL, recv_buf[RECV_MAX_LEN];
	int recv_len = 0;
	vector<vector<uint8_t>> send_data;
	auto mult_sync = CMultSync();
	while(1){
		send_data = scParse.get_send_data(received, recv_len);
		FOR(i, send_data.size()){
			if(send_data[i][0] == SC_CMD_DELAY){
				uint32_t delay_ms_num = (send_data[i][1])+(send_data[i][2]<<8)+
					(send_data[i][3]<<16)+(send_data[i][4]<<24);
				if(scParse.getDebug()){
					//mtx.lock(); debug(com); debug(": delay %d ms\n", delay_ms_num ); mtx.unlock();
				}
				delay(delay_ms_num);
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
			}else if(send_data[i][0] == SC_CMD_CUSTOM &&
					!memcmp(&send_data[i][1], "FLUSH", min(5, (int)send_data[i][0]))){
					btio.clear();
			}else if(send_data[i][0] == SC_CMD_CUSTOM &&
					!memcmp(&send_data[i][1], "PENDING", min(7, (int)send_data[i][0]))){
					mult_sync.pending();
			}else if(send_data[i][0] == SC_CMD_CUSTOM &&
					!memcmp(&send_data[i][1], "SYNC", min(4, (int)send_data[i][0]))){
					mult_sync.sync();
			}else if(send_data[i][0] == SC_CMD_CUSTOM &&
					!memcmp(&send_data[i][1], "DUMP_RAW", min(8, (int)send_data[i][0]))){
					string new_name = string(filename);
					new_name.resize(new_name.length()-3);
					new_name += string("_raw.sc");
					scParse.dump_2_file(new_name);
			}else if(send_data[i][0] == SC_CMD_CUSTOM &&
					!memcmp(&send_data[i][1], "SHARE", min(5, (int)send_data[i][0]))){
					char v[128]={0}, *s=(char*)&send_data[i][6];
					int max_len = send_data[i].size() - 6;
					while((*s == ' '||*s == ','||*s == ':') && s-(char*)&send_data[i][0]<send_data[i].size())s++;
					strncpy(v, s, send_data[i].size()-(s-(char*)&send_data[i][0]));
					char *e=v; while(isalnum(*e)||*e=='_')e++; *e = '\0';
					share_variable(scParse, v);
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

