#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hashmap.h"
#include "parse_init.h"
#include "pool_param.h"
#include "pool_format.h"
#include "hashmap.h"
#include "utils.h"

#define parse_debug printf

#define MAX_WARNING_NUM 64
static int warning_num;
static char* warning_list[MAX_WARNING_NUM];
static char* error_msg;
#define WARNING_MSG(...) { \
        if(warning_num < MAX_WARNING_NUM){ \
            char buf[1024] = {0}; \
            snprintf(buf, 1000, __VA_ARGS__); \
            warning_list[warning_num++] = strdup(buf); \
        } \
    }
#define ERROR_MSG(...) { \
        char buf[1024] = {0}; \
        snprintf(buf, 1000, __VA_ARGS__); \
        error_msg = strdup(buf); \
    }

static void init_param(struct csv_data* csv_p)
{
    int i;
    if (csv_p->line_num <= PARAM_RSV_LINE) {
        WARNING_MSG("No record in file %s", csv_p->filename);
    }
    struct param* cur_param = NULL;
    for (i = PARAM_RSV_LINE; i < csv_p->line_num; i++) {
        struct csv_line* line = &csv_p->lines[i];
        if (line->col_num < PARAM_MIN_COL) {
            WARNING_MSG("Not enough colume @ %s(line:%d)", csv_p->filename, i + 1);
            continue;
        }
        struct csv_cell* cell_name   = &line->cells[PARAM_NAME_COL];
        struct csv_cell* cell_width  = &line->cells[PARAM_WIDTH_COL];
        struct csv_cell* cell_type   = &line->cells[PARAM_TYPE_COL];
        struct csv_cell* cell_key    = &line->cells[PARAM_KEY_SUB_COL];
        struct csv_cell* cell_value  = &line->cells[PARAM_VAL_RNE_COL];
        struct csv_cell* cell_def    = &line->cells[PARAM_DEF_COL];
        struct csv_cell* cell_config = &line->cells[PARAM_CONFIG_COL];
        struct csv_cell* cell_output = &line->cells[PARAM_OUT_COL];
        if (cell_name->text[0] == '#') {
            continue;
        }
        char cell_pos[64];
        sprintf(cell_pos, "%s(line:%d)", csv_p->filename, i + 1);
        int flag_enum_add = false;
        if (strlen(cell_name->text) > 0) { // parameter define
            if (check_param_name(cell_name->text)) {
                ERROR_MSG("Invalid 'param_name' formating: '%s' @ %s", cell_name->text, cell_pos);
                return;
            } else if (param_get(cell_name->text)) { // check redefine
                int basic_type = type_idx(cell_type->text);
                if (basic_type == BTYPE_ENUM){
                    flag_enum_add = true;
                    cur_param = param_get(cell_name->text);
                } else {
                    ERROR_MSG("Redefined param name: '%s' @ %s", cell_name->text, cell_pos);
                    return;
                }
            }
            if (check_param_width(cell_width->text)) { // check bit width
                ERROR_MSG("Invalid 'bit_width' formating: '%s' @ %s", cell_width->text, cell_pos);
                return;
            } else if (check_param_type(cell_type->text)) { // check param type
                ERROR_MSG("Invalid 'type' formating: '%s' @ %s", cell_width->text, cell_pos);
                return;
            } else if (check_param_def(cell_def->text)) { // check default
                ERROR_MSG("Invalid 'default' formating: '%s' @ %s", cell_width->text, cell_pos);
                return;
            } else if (strlen(cell_config->text)) {
                if (check_param_cfg(cell_config->text)) { // check config flag
                    ERROR_MSG("Invalid 'cfg_flag' formating: '%s' @ %s", cell_config->text, cell_pos);
                    return;
                }
            }
            int basic_type = type_idx(cell_type->text);
            if (basic_type == BTYPE_ENUM || basic_type == BTYPE_BITMAP) { // type == enum/bitmap
                if (strlen(cell_key->text) == 0) {
                    ERROR_MSG("'sub_key' can NOT be empty @ %s", cell_pos);
                    return;
                } else if (strlen(cell_value->text) == 0) {
                    ERROR_MSG("'value/range' can NOT be empty @ %s", cell_pos);
                    return;
                } else if (check_param_cfg(cell_key->text)) {
                    ERROR_MSG("Invalid 'key' format: '%s' @ %s", cell_key->text, cell_pos);
                    return;
                } else if (check_param_cfg(cell_value->text)) {
                    ERROR_MSG("Invalid 'value' format: '%s' @ %s", cell_value->text, cell_pos);
                    return;
                }
            } else { // other type
                if (strlen(cell_key->text) && strcmp(cell_key->text, "0") && check_param_name(cell_key->text)) {
                    ERROR_MSG("Invalid 'sub_key' format: '%s' @ %s", cell_key->text, cell_pos);
                    return;
                } else if (strlen(cell_value->text) && check_param_range(cell_value->text)) {
                    ERROR_MSG("Invalid 'range' format: '%s' @ %s", cell_value->text, cell_pos);
                    return;
                }
            }
            // All check done, insert param to pool.
            int cfg = strlen(cell_config->text) ? str2u32(cell_config->text) : 0;
            if (basic_type == BTYPE_INVALID) {
                if (!param_get(cell_type->text)) { // check if param referenced is defined
                    ERROR_MSG("'%s' is undefined and not a basic @ %s", cell_type->text, cell_pos);
                    return;
                }
                if (strlen(cell_width->text) || strlen(cell_key->text) || strlen(cell_value->text) ||
                        strlen(cell_def->text) || strlen(cell_output->text)) {
                    WARNING_MSG("Reference type should leave cells as empty except 'name' & 'type' @ %s", cell_pos);
                }
                assert(param_alias(cell_type->text, cell_name->text, cell_pos) == POOL_PARAM_SUCCESS);
            } else {
                int bit_offset, bit_width, bit_length;
                char* width_str = NULL;
                if (strstr(cell_width->text, "/")) {
                    char* str = strdup(cell_width->text), *oo = str, *ww = NULL, *ll = NULL;
                    int i;
                    for (i = 0; i < strlen(cell_width->text) - 1; i++) {
                        if (str[i] == '/') {
                            if (!ww) {
                                ww = &str[i + 1];
                            } else {
                                ll = &str[i + 1];
                            }
                            str[i] = '\0';
                        }
                    }
                    bit_offset = str2u32(oo);
                    bit_width = str2u32(ww);
                    bit_length = str2u32(ll);
                    free(str);
                } else if (is_valid_num(cell_width->text)) {
                    bit_offset = 0;
                    bit_width = bit_length = str2u32(cell_width->text);
                } else {
                    bit_offset = bit_width = bit_length = 0;
                    width_str = cell_width->text;
                }
                if (bit_width != bit_length && bit_width > sizeof(uint32_t) * 8) {
                    ERROR_MSG("Width of the values in type '%s' must NOT lager than 32 bits @ %s", cell_type->text, cell_pos);
                    return;
                }
                if(flag_enum_add && cur_param){
                    flag_enum_add = false;
                } else {
                    cur_param = param_add(cell_name->text, bit_offset, bit_width, bit_length, width_str, basic_type,
                            cell_key->text, cell_value->text, cell_def->text, cell_output->text, "",
                            cfg, cell_pos);
                }
                if (basic_type == BTYPE_ENUM || basic_type == BTYPE_BITMAP) { // type == enum/bitmap
                    int res = param_enum_add(cur_param, cell_key->text, str2u32(cell_value->text), cell_output->text, cell_pos);
                    assert(res == POOL_PARAM_KEY_MUST_INC || res == POOL_PARAM_SUCCESS);
                    if (res == POOL_PARAM_KEY_MUST_INC) {
                        ERROR_MSG("The values in the enumeration must be in ascending order @ %s", cell_pos);
                        return;
                    }
                }
            }
        } else if (strlen(cell_key->text) && strlen(cell_value->text) && !strlen(cell_width->text)
                   && !strlen(cell_type->text)) { // enum subkey & value
            if (cur_param->basic_type != BTYPE_ENUM && cur_param->basic_type != BTYPE_BITMAP) {
                ERROR_MSG("enum/bitmap has no root param @ %s", cell_pos);
                return;
            } else if (check_param_name(cell_key->text)) {
                ERROR_MSG("Invalid name format: '%s' @ %s", cell_key->text, cell_pos);
                return;
            } else if (check_param_value(cell_value->text)) {
                ERROR_MSG("Invalid key format: '%s' @ %s", cell_value->text, cell_pos);
                return;
            }
            int res = param_enum_add(cur_param, cell_key->text, str2u32(cell_value->text), cell_output->text, cell_pos);
            assert(res == POOL_PARAM_KEY_MUST_INC || res == POOL_PARAM_SUCCESS);
            if (res == POOL_PARAM_KEY_MUST_INC) {
                ERROR_MSG("The values in the enumeration must be in ascending order @ %s", cell_pos);
                return;
            }
        } else {
            int i;
            for (i = 0; i < PARAM_MIN_COL; i++) {
                if (strlen(line->cells[i].text)) {
                    ERROR_MSG("Invalid input line @ %s", cell_pos);
                    return;
                }
            }
        }
    }
}

static void init_format(struct csv_data* csv_f)
{
    int i;
    if (csv_f->line_num <= FMT_RSV_LINE) {
        WARNING_MSG("No record in file %s", csv_f->filename);
    }
    struct format* cur_format = NULL;
    for (i = FMT_RSV_LINE; i < csv_f->line_num; i++) {
        struct csv_line* line = &csv_f->lines[i];
        if (line->col_num < FMT_MIN_COL) {
            WARNING_MSG("Not enough colume @ %s(line:%d)", csv_f->filename, i + 1);
            continue;
        }
        struct csv_cell* cell_cmd    = &line->cells[FMT_CMD_COL];
        struct csv_cell* cell_remark = &line->cells[FMT_REMARK_COL];
        struct csv_cell* cell_key    = &line->cells[FMT_KEY_COL];
        struct csv_cell* cell_param  = &line->cells[FMT_PARAMS_COL];
        if (cell_cmd->text[0] == '#') {
            continue;
        }
        char cell_pos[64];
        sprintf(cell_pos, "%s(line:%d)", csv_f->filename, i + 1);
        if (strlen(cell_cmd->text)) { // It's a first command line
            if (check_param_name(cell_cmd->text)) {
                ERROR_MSG("Invalid command define: '%s' @ %s", cell_cmd->text, cell_pos);
                return;
            }
            if (format_get(cell_cmd->text)) {
                //ERROR_MSG("Redefined format: '%s' @ %s", cell_cmd->text, cell_pos);
                cur_format = format_get(cell_cmd->text);
            } else {
                cur_format = format_add(cell_cmd->text, cell_pos); // add new format (without sub_items)
            }
            assert(cur_format);
        } else if (!strlen(cell_remark->text) && !strlen(cell_key->text) && !strlen(cell_param->text)) { // It's empty line
            continue;
        }
        if (check_param_value(cell_key->text)) {
            ERROR_MSG("Invalid key_code : '%s' @ %s", cell_key->text, cell_pos);
            return;
        } else if (!cur_format) {
            ERROR_MSG("Missing command defination @ %s", cell_pos);
            return;
        } else if (check_format_param(cell_param->text)) {
            ERROR_MSG("Invalid parameters format '%s' @ %s", cell_param->text, cell_pos);
            return;
        }
        int key_code = str2u32(cell_key->text);
        int res = format_item_add(cur_format, cell_remark->text, key_code, cell_param->text, cell_pos);
        assert(res == POOL_FORMAT_KEY_MUST_INC || res == POOL_FORMAT_SUCCESS);
        if (res == POOL_FORMAT_KEY_MUST_INC) {
            ERROR_MSG("The values in the enumeration must be in ascending order @ %s", cell_pos);
            return;
        }
    }
}

static int auto_define_formats(void* p, void* data)
{
    struct format* format = (struct format*)data;
    // auto define param which as only 1 item in format
    if (format->format_num == 1 && !param_get(format->name)) {
        param_add(format->name, 0, 0, 0, NULL, BTYPE_STREAM, "0", "", "", "", "", 0, format->pos);
    }
    return POOL_FORMAT_SUCCESS;
}
static void auto_define(void)
{
#define PROTO_ROOT        "PROTO_ALL"
#define PROTO_UNUSED      "Unused"
#define PROTO_UNFINISHED  "Unfinished"
    if (!format_get(PROTO_ROOT)) {
        ERROR_MSG("'%s' as ROOT entrance must be defined", PROTO_ROOT);
        return;
    }
    if (!param_get(PROTO_UNUSED)) {
        param_add(PROTO_UNUSED, 0, 0, 0, NULL, BTYPE_STREAM, "", "", "", "", "", (1 << CFG_UNUSED_POS), "No where");
    }
    if (!param_get(PROTO_UNFINISHED)) {
        param_add(PROTO_UNFINISHED, 0, 0, 0, NULL, BTYPE_STREAM, "", "", "", "", "", 0, "No where");
    }
    pool_format_iterate(auto_define_formats, NULL);
}

static int validation_params(void* p, void* data)
{
    struct param* param = (struct param*)data;
    if (param->basic_type == BTYPE_ENUM || param->basic_type == BTYPE_UNSIGNED ||
            param->basic_type == BTYPE_SIGNED || param->basic_type == BTYPE_T_0_625MS ||
            param->basic_type == BTYPE_T_1_25MS || param->basic_type == BTYPE_T_10MS) {
        if (param->bit_width > 32) {
            ERROR_MSG("Param '%s' bit_width must less than 32 @ %s", param->name, param->pos);
            return !POOL_PARAM_SUCCESS;
        }
    }
    if (param->basic_type != BTYPE_ENUM && param->basic_type != BTYPE_BITMAP) { // all other types
        if (strlen(param->key_str)) {
            if (strcmp(param->key_str, "0")) {
                struct param* p = param_get(param->key_str);
                if (!p) {
                    ERROR_MSG("Subkey '%s' in '%s' not defined @ %s", param->key_str, param->name, param->pos);
                    return !POOL_PARAM_SUCCESS;
                } else if (p->basic_type != BTYPE_ENUM) {
                    ERROR_MSG("Invalid type, subkey '%s' @%s used by '%s' @ %s must be enum type", p->name, p->pos, param->name,
                              param->pos);
                    return !POOL_PARAM_SUCCESS;
                }
            }
            if (!format_get(param->name)) {
                ERROR_MSG("Param '%s' with subkey '%s' not defined in format @ %s", param->name, param->key_str, param->pos);
                return !POOL_PARAM_SUCCESS;
            }
        }
    }
    if (param->bit_width > 0 && param->bit_offset + param->bit_width > param->bit_length) { // bit share param
        ERROR_MSG("Param '%s' bit width exceed max width @ %s", param->name, param->pos);
        return !POOL_PARAM_SUCCESS;
    }
    return POOL_PARAM_SUCCESS;
}
static int validation_formats(void* p, void* data)
{
    struct format* format = (struct format*)data;
    // format subItems validation
    int i, j, k;
    for (i = 0; i < format->format_num; i++) {
        struct format_item* item = &format->items[i];
        for (j = 0; j < item->params_num; j++) {
            struct format_param* p = &item->params[j];
            struct param* param = param_get(p->type);
            if (!param) {
                ERROR_MSG("Type '%s' in format '%s' not defined in param @ %s", p->type, format->name, item->pos);
                return !POOL_FORMAT_SUCCESS;
            } else if (param->basic_type != BTYPE_ENUM && param->basic_type != BTYPE_BITMAP &&
                       param->key_str && strlen(param->key_str) && strcmp(param->key_str, "0")) {
                int found = false;
                for (k = j - 1; k >= 0; k--) {
                    if (!strcmp(item->params[k].type, param->key_str)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ERROR_MSG("Subkey '%s' in type '%s' not defined in format list @ %s", param->key_str, p->type, item->pos);
                    return !POOL_FORMAT_SUCCESS;
                }
            }
            if (strcmp(p->type, p->name) && param_get(p->name)) {
                ERROR_MSG("Item name '%s' is already defined as a type name @ %s", p->name, item->pos);
                return !POOL_FORMAT_SUCCESS;
            }
        }
    }
    return POOL_FORMAT_SUCCESS;
}

static int unused_check_get_fmt(void* p, void* data)
{
    map_t map = (map_t)p;
    struct format* format = (struct format*)data;
    int i, j;
    for (i = 0; i < format->format_num; i++) {
        struct format_item* item = &format->items[i];
        for (j = 0; j < item->params_num; j++) {
            struct format_param* p = &item->params[j];
            hashmap_put(map, p->type, NULL);
        }
    }
    return MAP_OK;
}
static int unused_check_get_param(void* p, void* data)
{
    map_t map = (map_t)p;
    struct param* param = (struct param*)data;
    hashmap_put(map, param->key_str, NULL);
    return MAP_OK;
}
static int check_unused(void* p, void* data)
{
    map_t map = (map_t)p;
    struct param* param = (struct param*)data;
    any_t arg;
    if (hashmap_get(map, param->name, &arg) != MAP_OK) {
        if (strcmp(param->name, PROTO_ROOT) && strcmp(param->name, PROTO_UNUSED)) {
            WARNING_MSG("Unused parameter '%s' @ %s", param->name, param->pos);
        }
    }
    return MAP_OK;
}
static void check_unused_param(void)
{
    map_t param_map = hashmap_new();
    pool_param_iterate(unused_check_get_param, param_map);
    pool_format_iterate(unused_check_get_fmt, param_map);
    pool_param_iterate(check_unused, param_map);
}

static void validation(void)
{
    // params validation
    if (pool_param_iterate(validation_params, NULL)) {
        return;
    }
    // formats validation
    if (pool_format_iterate(validation_formats, NULL)) {
        return;
    }
    // unused check
    check_unused_param();
}

struct parse_res parse_init(void)
{
    int i;
    struct parse_res res = { &error_msg, !!error_msg, warning_list, warning_num };
    for (i = 0;; i++) {
        char fileNameFormat[32];
        char fileNameParam[32];
        struct csv_data format, param;
        sprintf(fileNameFormat, "format%d.csv", i);
        sprintf(fileNameParam, "param%d.csv", i);
        int f_res = csv_read(fileNameFormat, &format);
        int p_res = csv_read(fileNameParam, &param);
        if (f_res == CSV_SUCCESS && p_res == CSV_SUCCESS) {
            init_param(&param);
            csv_free(&param);
            if (error_msg) {
                res.error_num = !!error_msg;
                csv_free(&format);
                return res;
            }
            init_format(&format);
            csv_free(&format);
            if (error_msg) {
                res.error_num = !!error_msg;
                return res;
            }
        } else {
            csv_free(&param);
            csv_free(&format);
            break;
        }
    }
    auto_define();
    validation();
    res.error_num = !!error_msg;
    res.warning_num = warning_num;
    return res;
}

void parse_free(void)
{
    pool_param_free();
    pool_format_free();
}

__WEAK int check_param_name(char* s)
{
    return 0;
}
__WEAK int check_param_width(char* s)
{
    return 0;
}
__WEAK int check_param_type(char* s)
{
    return 0;
}
__WEAK int check_param_def(char* s)
{
    return 0;
}
__WEAK int check_param_cfg(char* s)
{
    return 0;
}
__WEAK int check_param_value(char* s)
{
    return 0;
}
__WEAK int check_param_range(char* s)
{
    return 0;
}
__WEAK int check_format_name(char* s)
{
    return 0;
}
__WEAK int check_format_value(char* s)
{
    return 0;
}
__WEAK int check_format_param(char* s)
{
    return 0;
}

