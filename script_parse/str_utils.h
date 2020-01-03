#ifndef __STR_UTILS_H__
#define __STR_UTILS_H__
#include "stdint.h"

enum{
	SC_SET_MODE,
	SC_SET_TOUT,
	SC_SET_DEBUG,
};

enum{
	SC_MODE_SEQUENCE,
	SC_MODE_CALLBACK,
};

enum{
	SC_DEF_RAND,
	SC_DEF_BYTE,
};

enum{
	SC_CMD_IMPORT,
	SC_CMD_DEFINE,
	SC_CMD_SEND,
	SC_CMD_RECV,
	SC_CMD_FOR,
	SC_CMD_LOOP,
	SC_CMD_SET,
	SC_CMD_DEBUG,
	SC_CMD_EXIT,
	SC_CMD_REMARK,
	SC_CMD_DELAY,
	SC_CMD_IGNORE,
};

enum{
	SC_VT_DEC,
	SC_VT_HEX,
	SC_VT_VAR,
	SC_VT_DEF,
};

typedef struct {
	int var;
	char var_str[16];
}sc_var_t;

typedef struct {
	char *name;
}sc_cmd_import_t ;

typedef struct {
	/* SC_DEF_RAND, SC_DEF_BYTE, */
	int type;
	char *name;
	uint8_t *data;
	int data_len;
	char *str_data;
}sc_cmd_define_t;

typedef struct {
	/* SC_VT_DEC, SC_VT_HEX, SC_VT_VAR, SC_VT_DEF, */
	int type;
	char *name;
	uint8_t *data_hex;
	int data_len;
	int data_dec;
	int name_len;
}sc_cmd_value_t;

typedef struct {
	sc_cmd_value_t *data_list;
	int len;
}sc_cmd_send_t;

typedef struct {
	sc_cmd_value_t *data_list;
	int len;
}sc_cmd_recv_t;

typedef struct {
	int type;
	char *name;
	uint8_t *data_hex;
	int width;
	int first;
	int second;
	char *str_width;
	char *str_first;
	char *str_second;
}sc_cmd_for_loop_t;

typedef struct {
	void *null;
}sc_cmd_loop_t;

typedef struct {
	int type;
	int data;
}sc_cmd_set_t;

typedef struct {
	sc_cmd_value_t *data_list;
	int len;
}sc_cmd_debug_t;

typedef struct {
	sc_cmd_value_t *data_list;
	int len;
}sc_cmd_exit_t;

typedef struct {
	sc_cmd_value_t *data_list;
	int len;
}sc_cmd_remark_t;

typedef struct {
	uint32_t delay_ms;
}sc_cmd_delay_t;

typedef struct {
	int type;
	union{
		sc_cmd_import_t import;
		sc_cmd_define_t define;
		sc_cmd_send_t send;
		sc_cmd_recv_t recv;
		sc_cmd_for_loop_t for_loop;
		sc_cmd_loop_t loop;
		sc_cmd_set_t set;
		sc_cmd_debug_t debug;
		sc_cmd_exit_t exit;
		sc_cmd_remark_t remark;
		sc_cmd_delay_t delay;
	};
}cmd_line_t;

cmd_line_t* parse_line(char *str);
void free_line(cmd_line_t* p);
void dump_line(cmd_line_t* p);

#endif /* __STR_UTILS_H__ */
