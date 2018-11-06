#ifndef __PARAM_H__
#define __PARAM_H__

#include "stdint.h"

enum {
	PARAM_ERR_NONE,
	PARAM_ERR_FORMAT,
	PARAM_ERR_NOT_EXIST,
	PARAM_ERR_INTERNAL,
	PARAM_ERR_OTHER,
};
#define DECLEAR_VAR_POOL(name) struct variable_pool *name = NULL;

struct param_line {
	char *title;
	uint32_t item_num;
	struct param_item *item;
};

struct variable_pool;

struct param_line* str2param(char *str);
struct param_line* create_param_hex(char *title, uint8_t *data, uint32_t len);
struct param_line* param_line_copy(struct param_line* pl);
int param_line_to_hex(struct param_line *pl, struct variable_pool *vp, uint8_t *buf);
int param_line_cmp(struct param_line*, struct variable_pool **vp, uint8_t *data, uint32_t len);
void param_add(struct variable_pool **vp, char *var_name, uint8_t *data, uint32_t len);
void free_param_line(struct param_line *pl);
void free_var_pool(struct variable_pool *vp);
void dump_param(struct param_line* pl);
void dump_var(struct variable_pool* vp);

#endif /* __PARAM_H__ */
