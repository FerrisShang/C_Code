#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "param.h"
#include "bt_usb.h"

#define dump(d, l) do{for(int _i=0;_i<l;_i++)printf("%02X ", (unsigned char)(d)[_i]);printf("\n");}while(0)

#define MAX_LEN (1ul<<14)

#define STR_RECV       "RECV"
#define STR_SEND       "SEND"
#define STR_REPEAT     "REPEAT"
#define STR_DELAY_MS   "DELAY_MS"

struct snoop_rep {
	struct snoop_rep* next;
	uint32_t idx;
	uint32_t repeat;
	uint32_t delay_ms;
	struct param_line *send;
	struct param_line *recv;
};

static struct snoop_rep *head;
static struct variable_pool *m_variable_pool;

int parse_snoop_txt(char *path, struct snoop_rep **head)
{
	struct snoop_rep *current;
	char buf[MAX_LEN];
	struct param_line *send;
	struct param_line *recv = create_param_hex("RECV", (uint8_t*)"\x04\x0e\x04\x01\x03\x0c\x00", 7);
	int rec_idx = 0, flag_recv_used;
	uint32_t repeat=-1, delay_ms=0;
	FILE *fp = fopen(path, "rb");
	*head = NULL;
	while(1){
		if(!fgets(buf, MAX_LEN, fp)){ break; }
		if(buf[0] == '#'){ continue; }

		struct param_line *pl = str2param(buf);
		if(pl == NULL){
			printf("Format error (%d):%s \n", rec_idx, buf);
			break;
		}
		if(!strcmp(pl->title, STR_RECV)){
			if(flag_recv_used != 1){
				printf("Warning: Dropping unused receiving param_line. idx=%d\n", rec_idx);
				free_param_line(recv);
			}
			recv = pl;
			flag_recv_used = 0;
		}else if(!strcmp(pl->title, STR_SEND)){
			send = pl;
			if(!*head){
				current = malloc(sizeof(struct snoop_rep));
				*head = current;
			}else{
				current->next = malloc(sizeof(struct snoop_rep));
				current = current->next;
			}
			current->next = NULL;
			current->idx = rec_idx;
			if(flag_recv_used == 1){
				current->recv = param_line_copy(recv);
			}else{
				current->recv = recv;
			}
			current->send = send;
			current->repeat = repeat;
			current->delay_ms = delay_ms;
			//Reset the config
			repeat=-1, delay_ms=0; flag_recv_used = 1;
		}else if(!strcmp(pl->title, STR_REPEAT)){
			//while(*p && !isdigit(*p)) p++;
			//repeat = atoi(p);
			free_param_line(pl);
		}else if(!strcmp(pl->title, STR_DELAY_MS)){
			//while(*p && !isdigit(*p)) p++;
			//delay_ms = atoi(p);
			free_param_line(pl);
		}else{
			printf("Unknown command(%d):%s \n", rec_idx, pl->title);
			free_param_line(pl);
			break;
		}
		rec_idx++;
	}
	fclose(fp);
	return rec_idx;
}

void running_snoop_txt(struct snoop_rep *head, uint8_t *data, uint32_t len)
{
	while(head){
		if(PARAM_ERR_NONE == param_line_cmp(head->recv, &m_variable_pool, data, len)){
			uint8_t send_buf[MAX_LEN];
			int send_len = param_line_to_hex(head->send, m_variable_pool, send_buf);
			if(send_len > 0){
				hci_send(send_buf, send_len);
			}
		}
		head = head->next;
	}
}

void drop_snoop(struct snoop_rep *head)
{
	if(head){
		free_param_line(head->send);
		head->send = NULL;
		free_param_line(head->recv);
		head->recv = NULL;
		if(head->next){
			drop_snoop(head->next);
		}
		free(head);
	}
}

void bt_recv_cb(uint8_t *data, int len)
{
	running_snoop_txt(head, data, len);
}
int main(void)
{

	hci_init(USB_LOG_ALL, bt_recv_cb);
	if(!parse_snoop_txt("snoop_rec.txt", &head)){
		printf("Parse snoop file failed!\n");
		return 0;
	}
	hci_send((uint8_t*)"\x01\x03\x0C\x00", 4);
	while(head)sleep(1);
	drop_snoop(head);
	free_var_pool(m_variable_pool);
	return 0;
}
