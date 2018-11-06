#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "assert.h"
#include "param.h"

#define CHECK_NULL(p) assert(p != NULL);
#define MAX_LEN (1ul<<14)
#define dump(d, l) do{for(int _i=0;_i<l;_i++)printf("%02X ", (unsigned char)(d)[_i]);printf("\n");}while(0)

enum {
	TYPE_VARIABLE,
	TYPE_PLACEHOLDER,
	TYPE_HEX_DATA,
};

struct placeholder{
	char *name;
	int32_t len;
};

struct item_var {
	char *name;
};

struct hex_data{
	uint32_t len;
	uint8_t *data;
};

struct param_item{
	uint8_t type;
	union {
		struct item_var var;
		struct placeholder plhd;
		struct hex_data hex;
	};
};

struct variable {
	char *name;
	uint32_t len;
	uint8_t *data;
};

struct variable_pool {
	struct variable_pool *next;
	struct variable var;
};

static int get_param_str_item_num(char *str)
{
	char buf[MAX_LEN];
	uint32_t buf_len = 0, num = 0;
	while(*str){ if(*str!=' '){buf[buf_len++]=*str++;}else{str++;} }
	buf[buf_len] = '\0';
	char *p_colon = strstr(buf, ":");
	char *p = p_colon ? p_colon+1 : buf;
	while(*p){
		if(buf_len>p-buf+2 && isxdigit(*p) && isxdigit(*(p+1))){
			while(buf_len>p-buf+2 && isxdigit(*p) && isxdigit(*(p+1))) p+=2;
			num++;
		}else if(*p == '{'){
			char *p_end = strstr(p, "}");
			if(p_end == NULL) return -1;
			p = p_end + 1;
			num++;
		}else{
			p++;
		}
	}
	return num;
}

static uint8_t h2d(char *h)
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

struct param_line* create_param_hex(char *title, uint8_t *data, uint32_t len)
{
	struct param_line *line = malloc(sizeof(struct param_line));
	CHECK_NULL(line);
	line->title = malloc(strlen(title)+1);
	CHECK_NULL(line->title);
	strcpy(line->title, title);
	line->item_num = 1;
	line->item = calloc(1, sizeof(struct param_item));
	CHECK_NULL(line->item);
	line->item[0].type = TYPE_HEX_DATA;
	line->item[0].hex.len = len;
	line->item[0].hex.data = malloc(len);
	CHECK_NULL(line->item[0].hex.data);
	memcpy(line->item[0].hex.data, data, len);
	return line;
}

struct param_line* param_line_copy(struct param_line* pl)
{
	assert(pl != NULL);
	struct param_line *line = malloc(sizeof(struct param_line));
	CHECK_NULL(line);
	line->title = malloc(strlen(pl->title)+1);
	CHECK_NULL(line);
	strcpy(line->title, pl->title);
	line->item_num = pl->item_num;
	line->item = calloc(line->item_num, sizeof(struct param_item));
	CHECK_NULL(line->item);
	for(int i=0;i<line->item_num;i++){
		line->item[i].type = pl->item[i].type;
		if(line->item[i].type == TYPE_VARIABLE){
			line->item[i].var.name = malloc(strlen(pl->item[i].var.name)+1);
		CHECK_NULL(line->item[i].var.name);
			strcpy(line->item[i].var.name, pl->item[i].var.name);
		}else if(line->item[i].type == TYPE_PLACEHOLDER){
			line->item[i].plhd.len = pl->item[i].plhd.len;
			line->item[i].plhd.name = malloc(strlen(pl->item[i].plhd.name)+1);
			CHECK_NULL(line->item[i].plhd.name);
			strcpy(line->item[i].plhd.name, pl->item[i].plhd.name);
		}else if(line->item[i].type == TYPE_HEX_DATA){
			line->item[i].hex.len = pl->item[i].hex.len;
			line->item[i].hex.data = malloc(pl->item[i].hex.len);
			CHECK_NULL(line->item[i].hex.data);
			memcpy(line->item[i].hex.data, pl->item[i].hex.data, pl->item[i].hex.len);
		}else{
		}
	}
	return line;
}

struct param_line* str2param(char *str)
{
	struct param_line *line;
	int item_num;
	if((item_num = get_param_str_item_num(str)) < 0){
		printf("Line format error: %s\n", str);
		return NULL;
	}
	line = malloc(sizeof(struct param_line));
	CHECK_NULL(line);
	line->item_num = item_num;
	line->item = calloc(item_num, sizeof(struct param_item));
	CHECK_NULL(line->item);

	char buf[MAX_LEN], *p = buf, *title;
	uint32_t buf_len = 0, item_idx = 0;
	while(*str){ if(*str!=' '){p[buf_len++]=*str++;}else{str++;} }
	p[buf_len] = '\0';
	while(*p){
		if((title = strstr(p, ":")) != NULL){
			line->title = calloc(1, title - buf + 1);
			memcpy(line->title, buf, title - buf);
			p = title + 1;
		}else if(buf_len>p-buf+2 && isxdigit(*p) && isxdigit(*(p+1))){
			uint8_t hex[MAX_LEN]; uint32_t hex_len = 0;
			do {
				hex[hex_len++] = h2d(p); p += 2;
			}while(buf_len>p-buf+2 && isxdigit(*p) && isxdigit(*(p+1)));
			line->item[item_idx].type = TYPE_HEX_DATA;
			line->item[item_idx].hex.len = hex_len;
			line->item[item_idx].hex.data = malloc(hex_len);
			memcpy(line->item[item_idx].hex.data, hex, hex_len);
			++item_idx;
		}else if(*p == '{'){
			char *p_end = strstr(p, "}");
			char *p_comma = strstr(p, ",");
			if(p_comma && p_comma < p_end){
				line->item[item_idx].type = TYPE_PLACEHOLDER;
				line->item[item_idx].plhd.len = atoi(p_comma+1);
				line->item[item_idx].plhd.name = calloc(1, p_comma - p);
				memcpy(line->item[item_idx].plhd.name, p+1, p_comma - p - 1);
			}else{
				line->item[item_idx].type = TYPE_VARIABLE;
				line->item[item_idx].var.name = calloc(1, p_end - p);
				memcpy(line->item[item_idx].var.name, p+1, p_end - p - 1);
			}
			p = p_end + 1;
			if(++item_idx == item_num) break;;
		}else{ p++; }
	}
	return line;
}

static int get_var(struct variable_pool *vp, char *var_name, uint8_t *buf)
{
	while(vp){
		if(!strcmp(vp->var.name, var_name)){
			memcpy(buf, vp->var.data, vp->var.len);
			return vp->var.len;
		}
		vp = vp->next;
	}
	return PARAM_ERR_NOT_EXIST;
}

static void merge_vp(struct variable_pool **vp1, struct variable_pool *vp2)
{
	if(vp2 == NULL) { return; }
	if(*vp1 == NULL) { *vp1 = vp2; return; }
	struct variable_pool *vp;
	while(vp2 != NULL){
		vp = *vp1;
		while(vp->next != NULL){
			if(!strcmp(vp->var.name, vp2->var.name)){
				break;
			}
			vp = vp->next;
		}
		struct variable_pool *t = vp2;
		vp2 = vp2->next;
		if(!strcmp(vp->var.name, t->var.name)){
			t->next = NULL;
			free_var_pool(t);
		}else{
			vp->next = t;
		}
	}
}

int param_line_to_hex(struct param_line *pl, struct variable_pool *vp, uint8_t *buf)
{
	int len = 0;
	for(int i=0;i<pl->item_num;i++){
		if(vp && pl->item[i].type == TYPE_VARIABLE){
			int l = get_var(vp, pl->item[i].var.name, &buf[len]);
			if(l > 0){ len += l; } else { return l; }
		}else if(vp && pl->item[i].type == TYPE_PLACEHOLDER){
			int l = get_var(vp, pl->item[i].plhd.name, &buf[len]);
			if(l > 0){ len += l; } else { return l;}
		}else if(pl->item[i].type == TYPE_HEX_DATA){
			memcpy(&buf[len], pl->item[i].hex.data, pl->item[i].hex.len);
			len += pl->item[i].hex.len;
		}else{
			return PARAM_ERR_INTERNAL;
		}
	}
	return len;
}

void param_add(struct variable_pool **vp, char *var_name, uint8_t *data, uint32_t len)
{
	struct variable_pool *p_vp;
	if(*vp == NULL){
		*vp = calloc(1, sizeof(struct variable_pool));
		CHECK_NULL(*vp);
		p_vp = *vp;
	}else if(!strcmp((*vp)->var.name, var_name)){
		p_vp = *vp;
	}else{
		char flag = 0;
		p_vp = *vp;
		while(p_vp->next != NULL){
			p_vp = p_vp->next;
			if(!strcmp(p_vp->var.name, var_name)){ flag = 1; break; }
		}
		if(!flag){
			p_vp->next = calloc(1, sizeof(struct variable_pool));
			CHECK_NULL(p_vp->next);
			p_vp = p_vp->next;
		}

	}
	if(p_vp->var.name != NULL){
		//p_vp->var.data = realloc(p_vp->var.data, len);
		free(p_vp->var.data);
		p_vp->var.data = malloc(len);
		CHECK_NULL(p_vp->var.data);
	}else{
		p_vp->next = NULL;
		p_vp->var.data = malloc(len);
		CHECK_NULL(p_vp->var.data);
		p_vp->var.name = malloc(strlen(var_name)+1);
		CHECK_NULL(p_vp->var.name);
	}
	p_vp->var.len = len;
	memcpy(p_vp->var.data, data, len);
	strcpy(p_vp->var.name, var_name);
}

void free_var_pool(struct variable_pool *vp)
{
	if(vp == NULL) { return; }
	if(vp->var.name){ free(vp->var.name); }
	if(vp->var.data){ free(vp->var.data); }
	if(vp->next){
		free_var_pool(vp->next);
	}
	free(vp);
}

int param_line_cmp(struct param_line *pl, struct variable_pool **vp, uint8_t *data, uint32_t len)
{
	uint32_t pos = 0;
	struct variable_pool *tmp_vp = NULL;
	int res = PARAM_ERR_NONE;
	assert(pl != NULL);
	for(int i=0;i<pl->item_num;i++){
		if(pl->item[i].type == TYPE_VARIABLE){
			uint8_t var_buf[MAX_LEN];
			uint32_t var_len = get_var(*vp, pl->item[i].var.name, var_buf);
			if(var_len < 0){
				res = PARAM_ERR_NOT_EXIST;
				break;
			}else if(pos + var_len <= len){
				if(!memcmp(var_buf, &data[pos], var_len)){
					pos += var_len;
				}else{
					res = PARAM_ERR_FORMAT;
					break;
				}
			}else{
				res = PARAM_ERR_FORMAT;
				break;
			}
		}else if(pl->item[i].type == TYPE_PLACEHOLDER){
			if(pos + pl->item[i].plhd.len <= len){
				param_add(&tmp_vp, pl->item[i].plhd.name, &data[pos], pl->item[i].plhd.len);
				pos += pl->item[i].plhd.len;
			}else{
				res = PARAM_ERR_FORMAT;
				break;
			}
		}else if(pl->item[i].type == TYPE_HEX_DATA){
			if(pos + pl->item[i].hex.len <= len &&
					!memcmp(pl->item[i].hex.data, &data[pos], pl->item[i].hex.len)){
				pos += pl->item[i].hex.len;
			}else{
				res = PARAM_ERR_FORMAT;
				break;
			}
		}
	}
	if(pos != len){
		free_var_pool(tmp_vp);
	}else{
		merge_vp(vp, tmp_vp);
	}
	return res;
}

void free_param_line(struct param_line *pl)
{
	if(pl){
		free(pl->title);
		for(int i=0;i<pl->item_num;i++){
			if(pl->item[i].type == TYPE_VARIABLE){
				free(pl->item[i].var.name);
			}else if(pl->item[i].type == TYPE_PLACEHOLDER){
				free(pl->item[i].plhd.name);
			}else if(pl->item[i].type == TYPE_HEX_DATA){
				free(pl->item[i].hex.data);
			}
		}
		free(pl->item);
		free(pl);
	}
}

void dump_param(struct param_line* pl)
{
	printf("Param title(%d): %s\n", pl->item_num, pl->title);
	for(int i=0;i<pl->item_num;i++){
		if(pl->item[i].type == TYPE_VARIABLE){
			printf("\t'V'(%s)\n", pl->item[i].var.name);
		}else if(pl->item[i].type == TYPE_PLACEHOLDER){
			printf("\t'P'(%s) : %d\n", pl->item[i].plhd.name, pl->item[i].plhd.len);
		}else if(pl->item[i].type == TYPE_HEX_DATA){
			printf("\t'H'(%d) : ", pl->item[i].hex.len); dump(pl->item[i].hex.data, pl->item[i].hex.len);
		}else printf("\tUnknown type!\n");
	}
}

void dump_var(struct variable_pool* vp)
{
	if(vp == NULL){ return; }
	printf("Dump variable pool:\n");
	do{
		printf("\t%s(%d): ", vp->var.name, vp->var.len);
		dump(vp->var.data, vp->var.len);
	}while(vp->next != NULL);
}
