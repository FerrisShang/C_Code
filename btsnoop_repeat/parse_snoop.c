#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#define MAX_LEN (1ul<<14)
#define dump(d, l) do{int _i;for(_i=0;_i<l;_i++)printf("%02X ", (unsigned char)d[_i]);printf("\n");}while(0)

struct snoop_rec {
	struct snoop_rec* next;
	uint32_t idx;
	uint32_t length;
	uint32_t flag;
	uint64_t timestamp;
	uint8_t ignore;
	uint8_t data[];
};

struct item_data {
	uint8_t data[1024];
	uint32_t len;
};

struct cmp_data {
	uint32_t item_num;
	struct item_data item_data[16];
};

static void bswap(char *dat,int len)
{
	char tmp;
	for(int i=0;i<len/2;i++){
		tmp = dat[i];
		dat[i] = dat[len-1-i];
		dat[len-1-i] = tmp;
	}
}

int is_data_match(struct cmp_data *cmp, uint8_t *buf, uint32_t buf_len)
{
	int remain_len = buf_len;
	for(int i=0;i<cmp->item_num;i++){
		if(cmp->item_data[i].len > 0){
			if(cmp->item_data[i].len > remain_len){
				return 0;
			}else if(!(i&1)){ //i==0?value:placeholder
				if(memcmp(cmp->item_data[i].data, &buf[buf_len - remain_len], cmp->item_data[i].len)){
					return 0;
				}
			}
		}else if(cmp->item_data[i].len == 0){
			return 1;
		}else{
			remain_len = -cmp->item_data[i].len;
		}
		remain_len -= cmp->item_data[i].len;
	}
	return 1;
}

int parse_snoop(char *path, struct snoop_rec **head, struct cmp_data *filter, uint32_t filter_len)
{
#define TIMEDIFF (0x00dcddb30f2f8000ULL)
	struct snoop_rec *current;
	char buf[MAX_LEN];
	int rec_idx = 0;
	FILE *fp = fopen(path, "rb");
	if(!fp){
		printf("File %s not found!\n", path);
		return 0;
	}
	*head = NULL;
	if(!fread(buf, sizeof("btsnoop"), 1, fp) || strcmp(buf, "btsnoop")){ return 0; }
	if(!fread(buf, 8, 1, fp) || memcmp(buf, "\x00\x00\x00\x01\x00\x00\x03\xEA", 8)){ return 0; }
	while(1){
		uint32_t item_len1, item_len2, flag, unused;
		uint64_t timestamp;
		if(!fread(&item_len1, sizeof(uint32_t), 1, fp) || !fread(&item_len2, sizeof(uint32_t), 1, fp) ||
				item_len1 != item_len2){ break; }
		bswap((char*)&item_len1, sizeof(uint32_t));
		if(!fread(&flag, sizeof(uint32_t), 1, fp)){ break; }
		if(!fread(&unused, sizeof(uint32_t), 1, fp)){ break; }
		if(!fread(&timestamp, sizeof(uint64_t), 1, fp)){ break; }
		if(!fread(buf, item_len1, 1, fp)){ break; }

		if(!*head){
			current = malloc(sizeof(struct snoop_rec) + item_len1);
			*head = current;
		}else{
			current->next = malloc(sizeof(struct snoop_rec) + item_len1);
			current = current->next;
		}
		bswap((char*)&timestamp, sizeof(uint64_t));
		current->next = NULL;
		current->idx = rec_idx;
		current->length = item_len1;
		current->flag = flag; //bit0: 0-send, 1-recv
		current->timestamp = timestamp - TIMEDIFF;
		current->ignore = 0;
		memcpy(current->data, buf, item_len1);
		for(int i=0;i<filter_len;i++){
			if(is_data_match(&filter[i], buf, item_len1)){
				current->ignore = 1;
				break;
			}
		}
		rec_idx++;
	}
	fclose(fp);
	return rec_idx;
}
int drop_snoop(struct snoop_rec *head)
{
	if(head){
		if(head->next){
			drop_snoop(head->next);
		}
		free(head);
	}
}

static char* d2h(uint8_t d)
{
	static char h[] = " xx";
	if((d>>4) >= 10){
		h[1] = 'A' + (d>>4) - 10;
	}else{
		h[1] = '0' + (d>>4);
	}
	if((d&0xF) >= 10){
		h[2] = 'A' + (d&0xF) - 10;
	}else{
		h[2] = '0' + (d&0xF);
	}
	return h;
}

static int is_all_digit(char *p)
{
	while(*p){ if(!isdigit(*p++)) return 0; } return 1;
}

void snoop_filter(struct snoop_rec *head, int argc, char *argv[])
{
	for(int i=0;i<argc;){
		if(!strcmp(argv[i], "-i") && i + 2 < argc && is_all_digit(argv[i+1]) && is_all_digit(argv[i+2])){
			struct snoop_rec *p = head;
			for(int j=0;j<atoi(argv[i+1])-1;j++){ if(!p){ return; } p = p->next; }
			for(int j=0;j<=atoi(argv[i+2]) - atoi(argv[i+1]);j++){ if(!p){ return; } p->ignore = 1; p = p->next; }
			i += 3;
		}else{
			i++;
		}
	}
}

void snoop_to_file(struct snoop_rec *head, char *path)
{
	uint8_t buf[MAX_LEN];
	FILE *fp = fopen(path, "w");
	while(head){
		if(!head->ignore){
			if((head->flag >> 24) & 0x1){
				strcpy(buf, "RECV:");
			}else{
				strcpy(buf, "SEND:");
			}
			for(int i=0;i<head->length;i++){
				strcat(buf, d2h(head->data[i]));
			}
			strcat(buf, "\n");
			fputs(buf, fp);
		}
		head = head->next;
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	char *file_in = argc > 1 ? argv[1] : "";
	char *file_out = "snoop_rec.txt";
	uint8_t buf[MAX_LEN];
	struct snoop_rec *head;
	struct cmp_data filter[] = {
		{2, {  {{0x04, 0x0F, 0x04, 0x00}, 4}, {"CMD status", 0}  }},
		{2, {  {{0x04, 0x13, 0x05, 0x01}, 4}, {"NubOfCmp", 0}  }},
		{2, {  {{0x01, 0x13, 0x20}, 3}, {"Update Param", 0}   }},
		{4, {  {{0x04, 0x3E}, 2}, {"L", 1}, {{0x03}, 1}, {"Update CMP", 0}   }},
	};
	if(!parse_snoop(file_in, &head, filter, sizeof(filter)/sizeof(filter[0]))){
		printf("Parse snoop file failed!\n");
		return 0;
	}
	snoop_filter(head, argc, argv);
	snoop_to_file(head, file_out);
	drop_snoop(head);
	return 0;
}
