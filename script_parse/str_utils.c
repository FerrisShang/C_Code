#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "str_utils.h"

static sc_var_t sc_com[] = {
	{ SC_CMD_IMPORT,  "IMPORT" },
	{ SC_CMD_DEFINE,  "DEFINE"  },
	{ SC_CMD_ASSIGN,  "ASSIGN"  },
	{ SC_CMD_SEND,    "SEND"    },
	{ SC_CMD_RECV,    "RECV"    },
	{ SC_CMD_FOR,     "FOR"     },
	{ SC_CMD_LOOP,    "LOOP"    },
	{ SC_CMD_SET,     "SET"     },
	{ SC_CMD_INFO,    "INFO"    },
	{ SC_CMD_DEBUG,   "DEBUG"   },
	{ SC_CMD_WARNING, "WARNING" },
	{ SC_CMD_ERROR,   "ERROR"   },
	{ SC_CMD_EXIT,    "EXIT"    },
	{ SC_CMD_REMARK,  "#"  },
	{ SC_CMD_DELAY,   "DELAY"   },
	{ SC_CMD_IGNORE,  "IGNORE"  },
	{ SC_CMD_CUSTOM,  "CUSTOM"  },
};

static sc_var_t sc_def[] = {
	{ SC_DEF_RAND, "RAND"},
	{ SC_DEF_BYTE, "BYTE"},
};

static sc_var_t sc_set[] = {
	{ SC_SET_MODE, "MODE"},
	{ SC_SET_TOUT, "TIMEOUT"},
	{ SC_SET_DEBUG,"DEBUG"},
};

static sc_var_t sc_mode[] = {
	{SC_MODE_SEQUENCE, "SEQUENCE"},
	{SC_MODE_CALLBACK, "CALLBACK"},
};

char* get_var_name(char *str, int *pos, char sep_char, int *str_len)
{
	assert(!pos || *pos < 1024);
	assert(str_len);
	char *s = str, *ret = NULL;
	if(pos) str = s + *pos;
	while(*str && isspace(*str)) str++;
	ret = str;
	while(*str && (*str=='_' || isalnum(*str))) str++;
	*str_len = str - ret;
	if(*str && sep_char && !isspace(*str) && *str != sep_char){ return NULL; }
	while(*str && isspace(*str)) str++;
	if(sep_char) while(*str && *str == sep_char) str++;
	if(pos){ *pos = str - s; }
	return ret;
}
int get_common(sc_var_t *sc_list, int sc_list_len, char *str, int *pos, char sep_char)
{
	assert(!pos || *pos < 1024);
	char *s = str;
	if(pos) str = s + *pos;
	while(*str && isspace(*str)) str++;
	for(int i=0;i<sc_list_len;i++){
		if(strstr(str, sc_list[i].var_str) == str){
			if(pos){
				str += strlen(sc_list[i].var_str);
				while(*str && isspace(*str)) str++;
				if(sep_char) while(*str && *str == sep_char) str++;
				*pos = str-s;
			}
			return sc_list[i].var;
		}
	}
	return -1;
}

int get_cmd(char *str, int *pos)
{
	return get_common(sc_com, sizeof(sc_com)/sizeof(sc_com[0]), str, pos, ':');
}

int get_mode(char *str, int *pos)
{
	return get_common(sc_mode, sizeof(sc_mode)/sizeof(sc_mode[0]), str, pos, 0);
}

int get_set(char *str, int *pos)
{
	return get_common(sc_set, sizeof(sc_set)/sizeof(sc_set[0]), str, pos, ',');
}

int get_def(char *str, int *pos)
{
	return get_common(sc_def, sizeof(sc_def)/sizeof(sc_def[0]), str, pos, ',');
}

char* get_var_hex(char *str, int *pos, char sep_char, int *str_len)
{
	assert(!pos || *pos < 1024);
	assert(str_len);
	char *s = str, *ret = NULL;
	if(pos) str = s + *pos;
	while(*str && isspace(*str)) str++;
	ret = str;
	while(*str && (*str==' ' || isxdigit(*str))) str++;
	*str_len = str - ret;
	if(*str && sep_char && !isspace(*str) && *str != sep_char){ return NULL; }
	if(sep_char) while(*str && *str == sep_char) str++;
	if(pos){ *pos = str - s; }
	return ret;
}
char* get_var_dec(char *str, int *pos, char sep_char, int *str_len)
{
	assert(!pos || *pos < 1024);
	assert(str_len);
	char *s = str, *ret = NULL;
	if(pos) str = s + *pos;
	while(*str && isspace(*str)) str++;
	ret = str;
	while(*str && (isdigit(*str)||*str=='-')) str++;
	*str_len = str - ret;
	if(*str && sep_char && !isspace(*str) && *str != sep_char){ return NULL; }
	if(sep_char) while(*str && *str == sep_char) str++;
	if(pos){ *pos = str - s; }
	return ret;
}

void get_var_def(char *str, int *pos, char sep_char, char **def, int *def_len, char **val, int *val_len)
{
	assert(!pos || *pos < 1024);
	assert(def && def_len && val && val_len);
	char *s = str, *ret = NULL;
	*def = NULL; *val = NULL; *def_len = 0; *val_len = 0;
	if(pos) str = s + *pos;
	while(*str && isspace(*str)) str++;
	ret = str;
	if(*str=='{'){ str++; }else{ return; }
	int len = 0, tpos = 0;
	*def = get_var_name(str, &tpos, 0, def_len);
	if(!*def) return;
	str += tpos;
	while(*str && isspace(*str)) str++;
	if(*str==','){ str++; }
	tpos = 0;
	*val = get_var_dec(str, &tpos, 0, val_len);
	str += tpos;
	while(*str && isspace(*str)) str++;
	if(*str=='}'){ str++; }else{ return; }
	if(sep_char) while(*str && *str == sep_char) str++;
	if(pos){ *pos = str - s; }
	return;
}

char* get_msg(char *str, int *pos, char sep_char, int *str_len)
{
	assert(!pos || *pos < 1024);
	assert(str_len);
	char *s = str, *ret = NULL;
	*str_len = 0;
	if(pos) str = s + *pos;
	while(*str && isspace(*str)) str++;
	ret = str;
	while(*str && isprint(*str) && *str!='{' && *str!='}') str++;
	*str_len = str - ret;
	if(*str && sep_char && !isspace(*str) && *str != sep_char){ return NULL; }
	if(sep_char) while(*str && *str == sep_char) str++;
	if(pos){ *pos = str - s; }
	return ret;
}

static void str2bytearray(char* str, int str_len, uint8_t *data, int *data_len)
{
	int idx = 0;
	for(int i=0;i<str_len;i++){
		if(isspace(str[i])){continue;}
		if(idx&1) data[idx/2] <<= 4;
		if(str[i] > '9') data[idx/2] |= ((str[i]|0x20) - 'a' + 10);
		else data[idx/2] |= (str[i] - '0');
		idx++;
	}
	*data_len = (idx+1)/2;
}

cmd_line_t* parse_line(char *str)
{
	int pos = 0, len; char *p;
	cmd_line_t *ret = (cmd_line_t*)calloc(1, sizeof(cmd_line_t)); assert(ret);
	strcpy(ret->raw, str);
	if(ret->raw[strlen(ret->raw)-1] != '\n'){ strcat(ret->raw, "\n"); }
	ret->type = get_cmd(str, &pos);
	//printf("\nT(%d), %s", ret->type, str);
	switch(ret->type){
		case SC_CMD_IMPORT:{
			p = get_msg(str, &pos, 0, &len);
			if(len > 0){
				ret->import.name = (char*)calloc(1, len+1); assert(ret->import.name);
				memcpy(ret->import.name, p, len);
			}else{
				assert(0); // import nothing
			}
			}break;
		case SC_CMD_ASSIGN:
		case SC_CMD_DEFINE:{
			p = get_var_name(str, &pos, ',', &len);
			if(len > 0){
				ret->define.name = (char*)calloc(1, len+1); assert(ret->define.name);
				memcpy(ret->define.name, p, len);
			}else{
				assert(0); // defined varible name invalid
			}
			ret->define.type = get_def(str, &pos);
			if(ret->define.type == SC_DEF_RAND){
				p = get_var_dec(str, &pos, 0, &len);
				if(len > 0){
					char str_len[16] = {0};
					memcpy(str_len, p, len);
					ret->define.data_len = atoi(str_len);
					ret->define.data = (uint8_t*)calloc(1, atoi(str_len)); assert(ret->define.data);
					for(int i=0;i<atoi(str_len);i++){
						ret->define.data[i] = rand();
					}
				}else{
					assert(0); // defined data invalid
				}
			}else if(ret->define.type == SC_DEF_BYTE){
				p = get_var_hex(str, &pos, 0, &len);
				if(len > 0){
					ret->define.str_data = (char*)calloc(1, len+1); assert(ret->define.str_data);
					ret->define.data = (uint8_t*)calloc(1, (len+1)/2); assert(ret->define.data);
					memcpy(ret->define.str_data, p, len);
					str2bytearray(p, len, ret->define.data, &ret->define.data_len);
				}else{
					assert(0); // defined data invalid
				}
			}else{
				assert(0); // define type error.
			}
			}break;
		case SC_CMD_IGNORE:
		case SC_CMD_SEND:
		case SC_CMD_RECV:{
			char data_found = true;
			sc_cmd_send_t *data = (sc_cmd_send_t*)&ret->send;
			data->len = 0;
			data->data_list = (sc_cmd_value_t*)calloc(1, sizeof(sc_cmd_value_t) * 32);
			while(data_found){
				data_found = false;
				p = get_var_hex(str, &pos, 0, &len);
				if(p && len){
					data_found = true;
					sc_cmd_value_t *value = &data->data_list[data->len++];
					value->type = SC_VT_HEX;
					value->data_hex = (uint8_t*)calloc(1, len);
					str2bytearray(p, len, value->data_hex, &value->data_len);
				}
				char *def=0; int def_len; char *val=0; int val_len;
				get_var_def(str, &pos, 0, &def, &def_len, &val, &val_len);
				if(def && def_len){
					data_found = true;
					sc_cmd_value_t *value = &data->data_list[data->len++];
					value->name_len = def_len;
					value->name = (char*)calloc(1, def_len+1);
					memcpy(value->name, def, def_len);
					if(val && val_len){
						value->type = SC_VT_DEF;
						char data_len[16] = {0};
						memcpy(data_len, val, val_len);
						value->data_len = atoi(data_len);
					}else{
						value->type = SC_VT_VAR;
						value->data_len = 0;
					}
				}
			}
			}break;
		case SC_CMD_FOR:{
			char *def, *val; int def_len, val_len;
			get_var_def(str, &pos, ',', &def, &def_len, &val, &val_len);
			if(def_len > 0 && val_len > 0){
				ret->for_loop.name = (char*)calloc(1, len+1); assert(ret->for_loop.name);
				memcpy(ret->for_loop.name, def, def_len);
				ret->for_loop.str_width = (char*)calloc(1, len+1); assert(ret->for_loop.str_width);
				memcpy(ret->for_loop.str_width, val, val_len);
				;
			}else{ assert(0); } // for_loop value error.
			p = get_var_dec(str, &pos, ',', &len);
			if(len > 0){
				ret->for_loop.str_first = (char*)calloc(1, len+1); assert(ret->for_loop.str_first);
				memcpy(ret->for_loop.str_first, p, len);
			}else{
				assert(0); // for_loop first data invalid
			}
			p = get_var_dec(str, &pos, 0, &len);
			if(len > 0){
				ret->for_loop.str_second = (char*)calloc(1, len+1); assert(ret->for_loop.str_second);
				memcpy(ret->for_loop.str_second, p, len);
			}else{
				assert(0); // for_loop second data invalid
			}
			ret->for_loop.data_hex = (uint8_t*)calloc(1, 16); assert(ret->for_loop.data_hex);
			ret->for_loop.width = atoi(ret->for_loop.str_width);
			ret->for_loop.first = atoi(ret->for_loop.str_first);
			ret->for_loop.second = atoi(ret->for_loop.str_second);
			}break;
		case SC_CMD_LOOP:{
			}break;
		case SC_CMD_SET:{
			ret->set.type = get_set(str, &pos);
			if(ret->set.type == SC_SET_MODE){
				ret->set.data = get_mode(str, &pos);
				assert(ret->set.data >= 0);
			}else if(ret->set.type == SC_SET_TOUT){
				p = get_var_dec(str, &pos, 0, &len);
				if(len > 0){
					char tout[32] = {0};
					memcpy(tout, p, len);
					ret->set.data = atoi(tout);
				}else{
					assert(0); // set data invalid
				}
			}else if(ret->set.type == SC_SET_DEBUG){
				p = get_var_dec(str, &pos, 0, &len);
				if(len > 0){
					char debug[32] = {0};
					memcpy(debug, p, len);
					ret->set.data = atoi(debug);
				}else{
					assert(0); // set data invalid
				}
			}else{
				assert(0); // set type error.
			}
			}break;
		case SC_CMD_CUSTOM:
		case SC_CMD_INFO:
		case SC_CMD_DEBUG:
		case SC_CMD_WARNING:
		case SC_CMD_ERROR:
		case SC_CMD_EXIT:{
			char data_found = true;
			sc_cmd_exit_t *data = (sc_cmd_exit_t*)&ret->exit;
			data->len = 0;
			data->data_list = (sc_cmd_value_t*)calloc(1, sizeof(sc_cmd_value_t) * 32);
			while(data_found){
				data_found = false;
				p = get_msg(str, &pos, 0, &len);
				if(p && len){
					data_found = true;
					sc_cmd_value_t *value = &data->data_list[data->len++];
					value->type = SC_VT_DEF;
					value->name = (char*)calloc(1, len+1);
					value->name_len = len;
					memcpy(value->name, p, len);
				}
				char *def=0; int def_len; char *val=0; int val_len;
				get_var_def(str, &pos, 0, &def, &def_len, &val, &val_len);
				if(def && def_len){
					data_found = true;
					sc_cmd_value_t *value = &data->data_list[data->len++];
					value->type = SC_VT_VAR;
					value->name_len = def_len;
					value->name = (char*)calloc(1, def_len+1);
					memcpy(value->name, def, def_len);
				}
			}
			}break;
		case SC_CMD_REMARK:{
			}break;
		case SC_CMD_DELAY:{
			p = get_var_dec(str, &pos, 0, &len);
			if(len > 0){
				char delay[32] = {0};
				memcpy(delay, p, len);
				ret->delay.delay_ms = atoi(delay);
			}else{
				assert(0); // set data invalid
			}
			}break;
		default:
			break;
			assert(0);
	}
	return ret;
}

void free_line(cmd_line_t* p)
{
	if(!p) return;
	switch(p->type){
		case SC_CMD_IMPORT:{
			if(p->import.name) free(p->import.name);
			}break;
		case SC_CMD_DEFINE:
		case SC_CMD_ASSIGN:{
			if(p->define.name) free(p->define.name);
			if(p->define.str_data) free(p->define.str_data);
			if(p->define.data) free(p->define.data);
			}break;
		case SC_CMD_IGNORE:
		case SC_CMD_SEND:
		case SC_CMD_RECV:
		case SC_CMD_CUSTOM:
		case SC_CMD_INFO:
		case SC_CMD_DEBUG:
		case SC_CMD_WARNING:
		case SC_CMD_ERROR:
		case SC_CMD_EXIT:{
			sc_cmd_send_t *data = (sc_cmd_send_t*)&p->send;
			for(int i=0;i<p->send.len;i++){
				sc_cmd_value_t *data = &(((sc_cmd_value_t*)p->send.data_list)[i]);
				free(data->data_hex);
				free(data->name);
			}
			free(p->send.data_list);
			}break;
		case SC_CMD_FOR:{
			if(p->for_loop.name) free(p->for_loop.name);
			if(p->for_loop.str_width) free(p->for_loop.str_width);
			if(p->for_loop.str_first) free(p->for_loop.str_first);
			if(p->for_loop.str_second) free(p->for_loop.str_second);
			if(p->for_loop.data_hex) free(p->for_loop.data_hex);
			}break;
		case SC_CMD_LOOP:{
			}break;
		case SC_CMD_SET:{
			}break;
		case SC_CMD_REMARK:{
			}break;
		default:
			break;
			assert(0);
	}
	free(p);
}
void dump_line(cmd_line_t* p)
{
	if(!p) return;
	printf("DUMP: ");
	switch(p->type){
		case SC_CMD_IMPORT:{
			printf("|IMPORT|%s|\n", p->import.name);
			}break;
		case SC_CMD_DEFINE:
		case SC_CMD_ASSIGN:{
			printf(p->type==SC_CMD_DEFINE?"|DEFINE|":"|ASSIGN|", p->import.name);
			if(p->define.type == SC_DEF_RAND){
				printf("RAND|");
			}else if(p->define.type == SC_DEF_BYTE){
				printf("BYTE|");
			}else{
				assert(0); // unsupported type
			}
			for(int i=0;i<p->define.data_len;i++){
				printf("%02X ", p->define.data[i]);
			}
			printf("\n");
			}break;
		case SC_CMD_IGNORE:
			if(p->type == SC_CMD_SEND) printf("|IGNORE|");
		case SC_CMD_SEND:
			if(p->type == SC_CMD_SEND) printf("|SEND|");
		case SC_CMD_RECV:{
			if(p->type == SC_CMD_RECV) printf("|RECV|");
			for(int i=0;i<p->send.len;i++){
				sc_cmd_value_t *data = &p->send.data_list[i];
				const char *str[] = {"DEC:", "HEX:", "VAR:", "DEF:"};
				printf("%s", str[data->type]);
				switch(data->type){
					case SC_VT_DEC:
					case SC_VT_HEX:
						for(int j=0;j<data->data_len;j++){
							printf("%02X ", data->data_hex[j]);
						}
						break;
					case SC_VT_VAR:
						printf("%s ", data->name);
						break;
					case SC_VT_DEF:
						printf("%s(%d) ", data->name, data->data_len);
						break;
				}
			}
			printf("\n");
			}break;
		case SC_CMD_FOR:{
			printf("|FOR|%s(%d),%d|%d|", p->for_loop.name, p->for_loop.width,
					p->for_loop.first, p->for_loop.second);
			printf("\n");
			}break;
		case SC_CMD_LOOP:{
			puts("|LOOP|");
			}break;
		case SC_CMD_SET:{
			printf("|SET|%s|%d|\n", p->set.type==SC_SET_MODE?"MODE":p->set.type==SC_SET_TOUT?"TIMEOUT":"DEBUG",
					p->set.data);
			}break;
		case SC_CMD_CUSTOM:
		case SC_CMD_INFO:
		case SC_CMD_DEBUG:
		case SC_CMD_WARNING:
		case SC_CMD_ERROR:
		case SC_CMD_EXIT:{
			if(p->type == SC_CMD_CUSTOM) printf("|CUSTOM|");
			if(p->type == SC_CMD_INFO) printf("|INFO|");
			if(p->type == SC_CMD_DEBUG) printf("|DEBUG|");
			if(p->type == SC_CMD_WARNING) printf("|WARNING|");
			if(p->type == SC_CMD_ERROR) printf("|ERROR|");
			if(p->type == SC_CMD_EXIT) printf("|EXIT|");
			for(int i=0;i<p->send.len;i++){
				sc_cmd_value_t *data = &p->send.data_list[i];
				const char *str[] = {"DEC:", "HEX:", "VAR:", "MSG:"};
				printf("%s", str[data->type]);
				switch(data->type){
					case SC_VT_VAR:
						printf("%s ", data->name);
						break;
					case SC_VT_DEF:
						printf("%s ", data->name);
						break;
					default:
						assert(0);
				}
			}
			printf("\n");
			}break;
		case SC_CMD_REMARK:{
			}break;
		default:
			assert(0);
	}
}

