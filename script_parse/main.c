#include "script_parse.h"

#define DUMP(d) \
	do{ \
		for(int i=0;i<d.size();i++){ \
			printf("<<<: "); \
			for(int j=0;j<d[i].size();j++){ \
				printf("%02X ", (uint8_t)d[i][j]); \
			} \
			puts(""); \
		} \
	}while(0)

int main(void)
{
	auto scParse = CScriptParse((char*)"demo.sc");
	auto send_data = scParse.get_send_data(NULL, 0);
	DUMP(send_data);
	send_data = scParse.get_send_data((uint8_t*)"\x01\x66\x88\x01", 4);
	DUMP(send_data);
	send_data = scParse.get_send_data((uint8_t*)"\x01\x66\x88\x01", 4);
	DUMP(send_data);
	send_data = scParse.get_send_data((uint8_t*)"\x01\x12\x54\x02", 4);
	DUMP(send_data);
	scParse.dump();
}


