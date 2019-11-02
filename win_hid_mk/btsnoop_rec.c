#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define TIMEDIFF (0x00dcddb30f2f8000ULL)
#define bswap(dat) _bswap((char*)&dat, sizeof(dat))
static void _bswap(char *dat,int len)
{
	int i;
	char tmp;
	for(i=0;i<len/2;i++){
		tmp = dat[i];
		dat[i] = dat[len-1-i];
		dat[len-1-i] = tmp;
	}
}

static long long* current_timestamp(void)
{
	static long long milliseconds;
	struct timeval te; 
	gettimeofday(&te, NULL);
	milliseconds = te.tv_sec*1000000LL + te.tv_usec + TIMEDIFF;
	bswap(milliseconds);
	return &milliseconds;
}

FILE *create_btsnoop_rec(char *path)
{
	char *id = "btsnoop";
	char version[] = {0x00,0x00,0x00,0x01};
	char type[] = {0x00,0x00,0x03,0xEA};
	FILE *fp = NULL;
	fp=fopen(path, "wb");
	if(fp == NULL)
		return NULL;
	fwrite(id, 8, 1, fp);
	fwrite(version, 4, 1, fp);
	fwrite(type, 4, 1, fp);
	return fp;
}

void close_btsnoop_rec(FILE *fp)
{
	if(fp == NULL)
		return;
	fflush(fp);
	fclose(fp);
}

void record_btsnoop(FILE *fp, char *hci_data, int data_len, char data_dir)
{
	int tLen = data_len;
	char drops[4] = {0,0,0,0};
	char flag[4] = {0,0,0,0};
	if(fp == NULL || hci_data == NULL)
		return;
	bswap(tLen);
	fwrite(&tLen, 4, 1, fp);
	fwrite(&tLen, 4, 1, fp);
	flag[3] = data_dir & 1;//0 - send; 1 -recv
	if(hci_data[0] == 1 || hci_data[0] == 4){
		flag[3] |= 0x02;
	}
	fwrite(&flag, 4, 1, fp);
	fwrite(&drops, 4, 1, fp);
	fwrite(current_timestamp(), sizeof(long long), 1, fp);
	fwrite(hci_data, data_len, 1, fp);
	fflush(fp);
}
