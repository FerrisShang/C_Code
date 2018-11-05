// http://sourceforge.net/projects/libusb-win32
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bt_usb.h"
#define dump(d, l) do{int _i;for(_i=0;_i<l;_i++)printf("%02X ", (unsigned char)d[_i]);printf("\n");}while(0)

#define MAX_LEN (1ul<<14)

#define STR_SEND       "SEND:"
#define STR_RECV       "RECV:"
#define STR_REPEAT     "REPEAT"
#define STR_REPEAT_END "REPEAT_END"

enum {
	CMD_SEND,
	CMD_RECV,
	CMD_REPEAT,
	CMD_REPEAT_END,
};

struct snoop_rep {
	struct snoop_rep* next;
	uint32_t idx;
	uint32_t command;
	uint32_t repeat;
	uint32_t rep_idx;
	uint32_t length;
	uint8_t data[];
};

struct snoop_repeat_stack {
	uint32_t idx;
	struct snoop_rep *p[32];
};

static uint8_t h2d(uint8_t *h)
{
	uint8_t d = 0;
	for(int i=0;i<2;i++){
		d <<= 4;
		if(h[i] >= 'A' && h[i] <= 'F'){
			d |= (h[i] - 'A' + 10);
		}else if(h[0] >= 'a' && h[0] <= 'f'){
			d |= (h[i] - 'a' + 10);
		}else{
			d |= (h[i] - '0');
		}
	}
	return d;
}

int parse_snoop_txt(char *path, struct snoop_rep **head)
{
	struct snoop_rep *current;
	char buf[MAX_LEN];
	int rec_idx = 0;
	FILE *fp = fopen(path, "rb");
	*head = NULL;
	while(1){
		uint32_t command, repeat;
		uint8_t data[MAX_LEN] = {0};
		if(!fgets(buf, MAX_LEN, fp)){ break; }
		uint8_t *p = buf;
		if(!memcmp(p, STR_SEND, sizeof(STR_SEND)-1)){
			command = CMD_SEND;
			p += sizeof(STR_SEND) - 1;
		}else if(!memcmp(p, STR_RECV, sizeof(STR_SEND)-1)){
			command = CMD_RECV;
			p += sizeof(STR_RECV) - 1;
		}else if(!memcmp(p, STR_REPEAT_END, sizeof(STR_REPEAT_END)-1)){
			command = CMD_REPEAT_END;
			p += sizeof(STR_REPEAT_END) - 1;
		}else if(!memcmp(p, STR_REPEAT, sizeof(STR_REPEAT)-1)){
			command = CMD_REPEAT;
			while(*p && !isdigit(*p)) p++;
			repeat = atoi(p);
		}else{
			printf("Unknown command:%s \n", p);
			break;
		}
		if(!*head){
			current = malloc(sizeof(struct snoop_rep) + strlen(p)+1);
			*head = current;
		}else{
			current->next = malloc(sizeof(struct snoop_rep) + strlen(p)+1);
			current = current->next;
		}
		current->next = NULL;
		current->idx = rec_idx;
		current->length = strlen(p)+1;
		current->command = command;
		current->repeat = repeat;
		memcpy(current->data, p, current->length);
		rec_idx++;
	}
	fclose(fp);
	return rec_idx;
}

void running_snoop_txt(struct snoop_rep *head)
{
	struct snoop_repeat_stack rep_stack = {0};
	while(head){
		uint8_t *p = head->data;
		uint32_t d_len = 0;
		uint8_t d_buf[MAX_LEN];
		while(*p){
			if((head->length > p-head->data+2) && isxdigit(*p) && isxdigit(*(p+1))){
				d_buf[d_len++] = h2d(p);
				p += 2;
			}else if(0){
				//TODO: process otherwise
			}else{
				p++;
			}
		}
		if(head->command == CMD_SEND){
			hci_send(d_buf, d_len);
		}else if(head->command == CMD_RECV){
			while(1){
				uint32_t recv_len, ep = d_buf[0] == 0x04 ? USB_EP_EVT_IN : USB_EP_ACL_IN;
				uint8_t buf[MAX_LEN];
				recv_len = hci_recv(buf, MAX_LEN, ep);
				if(recv_len <= 0){
					printf("recvived error: %d. wait for data:\n", recv_len);
					dump(d_buf, d_len);
					exit(0);
					//dump(head);
				}else if(!memcmp(d_buf, buf, recv_len < d_len ? recv_len : d_len)){
					break;
				}
			}
		}else if(head->command == CMD_REPEAT){
			head->rep_idx = 0;
			rep_stack.p[rep_stack.idx++] = head;
		}else if(head->command == CMD_REPEAT_END){
			if(rep_stack.idx <= 0){
				printf("Repeat stack error!\n");
				exit(0);
			}
			struct snoop_rep *rep_head = rep_stack.p[rep_stack.idx-1];
			rep_head->rep_idx++;
			if( rep_head->rep_idx < rep_head->repeat){ // repeat
				head = rep_head->next;
				continue;
			}else{
				rep_stack.idx--;
			}
		}
		head = head->next;
	}
}

int drop_snoop(struct snoop_rep *head)
{
	if(head){
		if(head->next){
			drop_snoop(head->next);
		}
		free(head);
	}
}

int main(void)
{
	struct snoop_rep *head;
	hci_init(USB_LOG_ALL, NULL);
	if(!parse_snoop_txt("snoop_rec.txt", &head)){
		printf("Parse snoop file failed!\n");
		return 0;
	}
	running_snoop_txt(head);
	drop_snoop(head);
	return 0;
}
