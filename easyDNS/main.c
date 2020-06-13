#include <stdio.h>
#include "easyDNS.h"

uint32_t callback(char* domain)
{
	fprintf(stderr, "%s\n", domain);
	fflush(stderr);
	if(strstr(domain, "localhost")) return 0x7F000001;
	return 0;
}
int main(void)
{
	easyDNS_t *server = easyDNS_init(callback);
	easyDNS_run(server);
}
