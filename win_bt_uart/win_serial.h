#ifndef __WIN_SERIAL_H__
#define  __WIN_SERIAL_H__

#include <windows.h>
#include <sys/time.h>
#include <thread>
#include <vector>
#include "squeue.h"
using namespace std;
#define SERIAL_SUCCESS         0
#define SERIAL_OPEN_FAILED    -1
#define SERIAL_ALREADY_OPENED -2
#define SERIAL_OTHER_ERROR    -9

class CSerial{
	public:
		HANDLE hComm;
		int recv_tout;
		CSerial(){hComm = 0;}
		int open(char *port, int baudrate, int recv_tout=200){
			this->recv_tout = recv_tout;
			int Status;
			char port_str[16] = "\\\\.\\";
			strcat(port_str, port);
			if(hComm){ return SERIAL_ALREADY_OPENED; }
			hComm = CreateFile(port_str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if(hComm == INVALID_HANDLE_VALUE){ hComm = 0; return SERIAL_OPEN_FAILED; }
			// Baudrate config
			DCB dcbSerialParams = {0};
			dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
			Status = GetCommState(hComm, &dcbSerialParams);
			if(Status != 1) return SERIAL_OTHER_ERROR;
			dcbSerialParams.BaudRate = baudrate;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;
			SetCommState(hComm, &dcbSerialParams);
			// Timeout config
			COMMTIMEOUTS timeouts = {0};
			timeouts.ReadIntervalTimeout = recv_tout;
			timeouts.ReadTotalTimeoutConstant = recv_tout;
			timeouts.ReadTotalTimeoutMultiplier = recv_tout;
			timeouts.WriteTotalTimeoutConstant = 1024;
			timeouts.WriteTotalTimeoutMultiplier = 1024;
			SetCommTimeouts(hComm, &timeouts);
			return SERIAL_SUCCESS;
		}
		int open(int port_num, int baudrate, int recv_tout=200){
			char port_str[16];
			sprintf(port_str, "\\\\.\\COM%d", port_num);
			return open(port_str, baudrate);
		}
		void close(void){
			if(!hComm) return;
			CloseHandle(hComm);
			hComm = 0;
		}
		inline void write(const uint8_t *data, int len){
			DWORD num;
			if(hComm){
				WriteFile(hComm, data, len, &num, NULL);
			}
		}
		int read(uint8_t *buf, int len){
			if(hComm){
				DWORD read_len = (DWORD)-1;
				COMSTAT stat;

				int retry = recv_tout;
				while(retry--){
					ClearCommError(hComm, NULL, &stat);
					if(stat.cbInQue >= len) break;
					Sleep(1);
				}
				if(stat.cbInQue >= len){ ReadFile(hComm, buf, len, &read_len, NULL); }
				return read_len;
			}
		}
		static vector<int> scan_port(void){
			char port_str[16]; HANDLE hComm; int i; vector<int> res;
			for(i=1;i<100;i++){
				sprintf(port_str, "\\\\.\\COM%d", i);
				hComm = CreateFile(port_str, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
				if(hComm != INVALID_HANDLE_VALUE){ res.push_back(i); CloseHandle(hComm); }
			}
			return res;
		}
};

class CBtIO: CSerial{
	bool exit_flag;
	CSerial serial;
	CSQueue<vector<uint8_t>> packages;
	thread *read_thread;
	void (*callback)(vector<uint8_t>& data, void *param);
	void *cb_param;
	public:
	int start(char *port, int baudrate){
		int res = open(port, baudrate, 50);
		if(res != SERIAL_SUCCESS){ return res; }
		if(!read_thread){ read_thread = new thread(__read__, this); }
		while(exit_flag);
		return SERIAL_SUCCESS;
	}
	int start(int port_num, int baudrate){
			char port_str[16];
			sprintf(port_str, "\\\\.\\COM%d", port_num);
			return start(port_str, baudrate);
	}
	int stop(void){
		exit_flag = true;
		close();
		if(read_thread){ read_thread->join(); delete read_thread; read_thread = NULL; }
	}
	CBtIO(){
		exit_flag = true;
		read_thread = NULL;
		callback = NULL;
	}
	void setCallback(void (*cb)(vector<uint8_t>& data, void *param), void *param){ callback = cb; cb_param = param; }
	int recv(uint8_t *buf, int max_len=1024, int tout=0){
		try{
			vector<uint8_t> pkt = packages.pop(tout);
			memcpy(buf, &pkt[0], min(max_len, (int)pkt.size()));
			return min(max_len, (int)pkt.size());
		}catch(...){
			return -1;
		}
	}
	int peek(uint8_t *buf, int max_len=1024){
		if(packages.size()){
			vector<uint8_t> pkt = packages.front();
			memcpy(buf, &pkt[0], min(max_len, (int)pkt.size()));
			return min(max_len, (int)pkt.size());
		}else{
			return -1;
		}
	}
	void clear(void){ packages.clear(); }
	int send(uint8_t *buf, int len){ write(buf, len); }
	int send(const vector<uint8_t>& data){ write((uint8_t*)&data[0], data.size()); }
#define ERR_NO_ERR 0
#define ERR_FMT   -1
#define ERR_TOUT  -2
	static int __get_stream__(CBtIO *btio, uint8_t *buf){
		int len = btio->read(buf, 1);
		if(len <= 0) return ERR_TOUT;
		if(buf[0] == 0x05){ // AHI
			len = btio->read(&buf[1], 8);
			if(len != 8) return ERR_TOUT;
			int left_len = *(uint16_t*)&buf[7];
			if(left_len > 1000) return ERR_FMT;
			len = btio->read(&buf[9], left_len);
			if(len != left_len) return ERR_TOUT;
			return 9+len;
		}else if(buf[0] == 0x04){ // HCI_EVT
			len = btio->read(&buf[1], 2);
			if(len != 2) return ERR_TOUT;
			int left_len = *(uint8_t*)&buf[2];
			len = btio->read(&buf[3], left_len);
			if(len != left_len) return ERR_TOUT;
			return 3+len;
		}else if(buf[0] == 0x02){ // HCI_ACL
			len = btio->read(&buf[1], 4);
			if(len != 4) return ERR_TOUT;
			int left_len = *(uint16_t*)&buf[3];
			len = btio->read(&buf[5], left_len);
			if(len != left_len) return ERR_TOUT;
			return 5+len;
		}else if(buf[0] == 0x01){ // HCI_CMD
			return ERR_FMT; // Host never receive HCI_CMD
		}
	}
	static void __read__(CBtIO *btio){
		btio->exit_flag = false;
		while(1){
			vector<uint8_t> buf;
			buf.resize(1024); buf[0] = -1;
			int res = __get_stream__(btio, &buf[0]);
			if(res > ERR_NO_ERR){
				buf.resize(res);
				if(btio->callback){ btio->callback(buf, btio->cb_param); }
				else{ btio->packages.push(buf); }
			}
			else if(res == ERR_FMT || res == ERR_TOUT){
				if(btio->exit_flag){
					return;
				}
			}else{
			}
		}
	}
};
#endif /* __WIN_SERIAL_H__ */

