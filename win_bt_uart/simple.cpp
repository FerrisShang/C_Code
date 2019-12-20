#include "squeue.h"
#include "win_serial.h"
#include "stdio.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	CBtIO btio;
	btio.start(4, 115200);

	btio.send((uint8_t*)"\x01\x03\x0c\x00", 4);
	uint8_t buf[1024];
	int len = btio.recv(buf, 1024, 1000);
	printf("len=%d\n", len);
	len = btio.recv(buf, 1024, 1000);
	printf("len=%d\n", len);
}

