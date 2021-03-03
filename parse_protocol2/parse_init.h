#ifndef __PARSE_INIT_H__
#define __PARSE_INIT_H__

#define PARAM_RSV_LINE 1
#define FMT_RSV_LINE   1

enum {
    PARAM_NAME_COL,
    PARAM_WIDTH_COL,
    PARAM_TYPE_COL,
    PARAM_KEY_SUB_COL,
    PARAM_VAL_RNE_COL,
    PARAM_DEF_COL,
    PARAM_CONFIG_COL,
    PARAM_OUT_COL,
    PARAM_DESC_COL,
    PARAM_MIN_COL = PARAM_DESC_COL,
};

enum {
    FMT_CMD_COL,
    FMT_REMARK_COL,
    FMT_KEY_COL,
    FMT_PARAMS_COL,
    FMT_MIN_COL = FMT_PARAMS_COL,
};

struct parse_res {
    char** error_msg;
    int error_num;
    char** warning_msg;
    int warning_num;
};

struct parse_res parse_init(void);
void parse_free(void);

int check_param_name(char* s); // non 0 means failed
int check_param_width(char* s);
int check_param_type(char* s);
int check_param_def(char* s);
int check_param_cfg(char* s);
int check_param_value(char* s);
int check_param_range(char* s);
int check_format_name(char* s);
int check_format_value(char* s);
int check_format_param(char* s);

#endif /* __PARSE_INIT_H__ */
