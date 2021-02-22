#include <stdio.h>
#include <string>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <windows.h>
#include <sys/time.h>
#include "parse_protocol.h"

using namespace std;
mutex mtx;


char* getTimeStr(char* buf)
{
	struct timeval tv; time_t t;
	gettimeofday(&tv, NULL); time(&t);
	strftime (buf, 128, "%x %X", localtime(&t));
	sprintf(buf, "%s.%03d", buf, tv.tv_usec / 1000);
	return buf;
}
#if 0
void parseUdp(void* p, int udpPort)
{
	vector<uint8_t> old_data, new_data;
	WSADATA       wsaData;
	SOCKET        ReceivingSocket;
	SOCKADDR_IN   ReceiverAddr;
	char          ReceiveBuf[4096];
	int           BufLength = 4096;
	SOCKADDR_IN   SenderAddr;
	int           SenderAddrSize = sizeof(SenderAddr);
	WSAStartup(MAKEWORD(2,2), &wsaData);
	ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(udpPort);
	ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));
    while (1) {
			int len = recvfrom(ReceivingSocket, ReceiveBuf, BufLength, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
			if(len > 0){
				new_data.resize(len);
				memcpy(&new_data[0], ReceiveBuf, len);
				if(old_data != new_data && new_data.size() > 0){
					mtx.lock();
					auto rec = ((CParseProtocol*)p)->unpack(new_data);;
					char buf[128]; printf("\n****** %s *****  %d  ***************************************\n",
									getTimeStr(buf), udpPort);
					for(int i=0;i<new_data.size();i++){ printf("%02X ", new_data[i]); } printf("\n");
					rec.dump();
					mtx.unlock();
				}
				old_data = new_data;
			}
    }
    //closesocket(socketS);
}
#endif
static string GetClipboardText()
{
  if (!OpenClipboard(nullptr)){ return string(""); }
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData == nullptr){ CloseClipboard(); return string(""); }
  char * pszText = static_cast<char*>( GlobalLock(hData) );
  if (pszText == nullptr){ CloseClipboard(); return string(""); }
  string text( pszText );
  GlobalUnlock( hData );
  CloseClipboard();
  return text;
}
void parseClipboard(void* p)
{
	vector<uint8_t> old_data, new_data;
	while(1){
		Sleep(300);
		new_data = CUnpack::s2b(GetClipboardText());
		if(old_data != new_data && new_data.size() > 0){
			mtx.lock();
			auto rec = ((CParseProtocol*)p)->unpack(new_data);;
			char buf[128]; printf("\n****** %s ********************************************\n", getTimeStr(buf));
			for(int i=0;i<new_data.size();i++){ printf("%02X ", new_data[i]); } printf("\n");
			rec.dump();
			mtx.unlock();
		}
		old_data = new_data;
	}

}
int main(int argc, char *argv[])
{
	CParseProtocol p;
	if(p.getStatus()){ return -1; }
	SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	thread th_clipboard(parseClipboard, &p);
	if(argc == 1){ th_clipboard.join(); return 0; }
	thread th_udp[argc-1];
	for(int i=0;i<argc-1;i++){
		//th_udp[i] = thread(parseUdp, &p, atoi(argv[i+1]));
	}
	th_clipboard.join();
}

