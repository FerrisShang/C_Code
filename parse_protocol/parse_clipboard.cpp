#include <stdio.h>
#include <string>
#include <windows.h>
#include "parse_protocol.h"

using namespace std;
string GetClipboardText()
{
  if (! OpenClipboard(nullptr)){ return string(""); }
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData == nullptr){ CloseClipboard(); return string(""); }
  char * pszText = static_cast<char*>( GlobalLock(hData) );
  if (pszText == nullptr){ CloseClipboard(); return string(""); }
  string text( pszText );
  GlobalUnlock( hData );
  CloseClipboard();
  return text;
}

int main(int argc, char *argv[])
{
	CParseProtocol p;
	if(p.getStatus()){ return -1; }
	vector<uint8_t> old_str, new_str;
	SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	while(1){
		Sleep(300);
		new_str = CUnpack::s2b(GetClipboardText());
		if(old_str != new_str && new_str.size() > 0){
			auto rec = p.unpack(new_str);;
			printf("\n***********************************************************************\n");
			for(int i=0;i<new_str.size();i++){ printf("%02X ", new_str[i]); } printf("\n");
			rec.dump();
		}
		old_str = new_str;
	}
}

