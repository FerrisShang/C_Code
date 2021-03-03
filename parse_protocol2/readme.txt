@startuml
[parse_protocol] -- [Param Module]
[parse_protocol] -- [Unpack Module]
[Param Module] -- [CSV Reader]
[Param Module] -- [Format Pool]
[Param Module] -- [Param Pool]
[Unpack Module] -- [Basic Type]
[Unpack Module] -- [Format Pool]
[Unpack Module] -- [Param Pool]
@enduml
/************************************************************************************************************************/
@startuml
class parse_protocol << (M,#80F0F0) >>{
struct parsed_item
struct parse_init_result
struct parse_init_result* parse_init(void)
struct parsed_data* parse_data(uint8_t data[], int length)
}
class "struct parsed_item" << (S,#40A0F0) >>{
	int out_priority;
	uint8_t *data;
	int data_len;
	char *title;
	char *item[];
	int item_num;
}
class "struct parsed_data" << (S,#40A0F0) >>{
	int items_num
	struct parsed_item *items
}
class "struct parse_init_result" << (S,#40A0F0) >>{
	char **error_msg;
	int error_num;
	char **warning_msg;
	int warning_num;
}
parse_protocol:struct parse_init_result* parse_init(void) --* "struct parse_init_result"
parse_protocol --* "struct parsed_data"
"struct parsed_data" --* "struct parsed_item"
@enduml
/************************************************************************************************************************/
@startuml
class csv_reader << (M,#80F0F0) >>{
	int csv_read(char* filename, struct csv_data* data);
	void csv_free(struct csv_data* data);
	void csv_dump(struct csv_data* data);
}
class "struct csv_cell" << (S,#40A0F0) >>{
	int x
	int y
	char *text;
}
class "struct csv_line" << (S,#40A0F0) >>{
	int col_num
	struct csv_cell *cell;
}
class "struct csv_data" << (S,#40A0F0) >>{
	int line_num;
	struct csv_line *lines;
}
class "enum csv_state" << (E,#40A0F0) >>{
	CSV_SUCCESS
	CSV_FILE_NOT_FOUND
}
csv_reader  --* "enum csv_state"
csv_reader  --* "struct csv_data"
"struct csv_data"  --* "struct csv_line"
"struct csv_line"  --* "struct csv_cell"
@enduml
/************************************************************************************************************************/
@startuml
class basic_type<< (M,#80F0F0) >>{
    int output_get(int basic_type, const uint8_t* data, int bit_len, char*** pp_out, int* out_num, const char* (*enum_str_cb)(int key, void* p), void* enum_p);
    void output_free(char** pp_out, int out_num);
    bool is_basic_type(char* str);
    const char* type_str(int basic_type);
    int type_idx(const char* type);
}
class "enum basic_type" << (E,#40A0F0) >>{
    BTYPE_ENUM,
    BTYPE_BITMAP,
    BTYPE_UNSIGNED,
    BTYPE_SIGNED,
    BTYPE_STREAM,
    BTYPE_HEX,
    BTYPE_ADDRESS,
    BTYPE_T_0_625MS,
    BTYPE_T_1_25MS,
    BTYPE_T_10MS,
    BTYPE_MAX,
    BTYPE_TRUNCATED = BTYPE_MAX,
    BTYPE_INVALID = -1,
}
class "enum basic_state" << (E,#40A0F0) >>{
    BTYPE_SUCCESS,
    BTYPE_FAILED,
}
basic_type --* "enum basic_state"
basic_type --* "enum basic_type"
@enduml
/************************************************************************************************************************/
@startuml
class pool_param << (M,#80F0F0) >>{
	struct param* param_add(char* name, int bit_offset, int bit_width, int bit_length, char* width_name, int basic_type, char* key_str, char* range_str, char* default_str, char* output, char* description, int cfg_flag, char* pos);
	int param_enum_add(struct param* p, char* subkey, int value, char* output, char* pos);
	int param_alias(char* old_name, char* new_name, char* pos);
	struct param* param_get(char* str);
	int pool_param_iterate(int (*callback)(void* p, void* data), void* p);
	void pool_param_dump(void);
	void pool_param_free(void);
}
class "enum state" << (E,#40A0F0) >>{
    POOL_PARAM_SUCCESS,
    POOL_PARAM_REDEFINE,
    POOL_PARAM_KEY_MUST_INC,
    POOL_PARAM_ERR_FMT,
    POOL_PARAM_ERR_TYPE,
    POOL_PARAM_ERR_MEM,
}
class "enum config_flag" << (E,#40A0F0) >>{
    CFG_OUTPUT_SUBKEY_POS     = 0,
    CFG_OUTPUT_SUBKEY_MASK    = 0x01,
    CFG_INC_INDENT_POS        = 1,
    CFG_INC_INDENT_MASK       = 0x01,
    CFG_OUT_BIT_IS_0_POS      = 2,
    CFG_OUT_BIT_IS_0_MASK     = 0x01,
    CFG_PARAM_CAN_LONGER_POS  = 3,
    CFG_PARAM_CAN_LONGER_MASK = 0x01,
    CFG_LENTGH_REF_POS        = 4,
    CFG_LENTGH_REF_MASK       = 0x01,
    CFG_UNUSED_POS            = 5,
    CFG_UNUSED_MASK           = 0x01,
    CFG_OUTPUT_PRIORITY_POS   = 8,
    CFG_OUTPUT_PRIORITY_MASK  = 0x0F,
}
class "struct param" << (S,#40A0F0) >>{
    char* name;
    int bit_offset;
    int bit_width;
    int bit_length;
    char* width_name;
    int basic_type;
    int enum_num;
    struct enum_item* enum_items;
    char* key_str;
    char* range_str;
    char* default_str;
    char* output;
    char* description;
    int cfg_flag;
    int cfg_output_subkey;
    int cfg_inc_indent;
    int cfg_output_bit_is0;
    int cfg_param_can_longer;
    int cfg_length_ref;
    int cfg_unused;
    int cfg_out_priority; /* -8 ~ 7*/
    char* pos; // param position
    int flag_alias;
}
pool_param --* "struct param"
pool_param --* "enum state"
"struct param" --* "enum config_flag"
@enduml
/************************************************************************************************************************/
@startuml
class pool_format << (M,#80F0F0) >>{
    struct format* format_add(char* name, char* pos);
    int format_item_add(struct format* p, char* remark, int key_code, char* param_list, char* pos);
    struct format* format_get(char* str);
    struct format_item* format_item_get(char* str, int key_code);
    void pool_format_free(void);
    int pool_format_iterate(int (*callback)(void* p, void* data), void* p);
    void pool_format_dump(void);
}
class "enum state" << (E,#40A0F0) >>{
    POOL_FORMAT_SUCCESS,
    POOL_FORMAT_REDEFINE,
    POOL_FORMAT_KEY_MUST_INC,
    POOL_FORMAT_ERR_FMT,
    POOL_FORMAT_ERR_TYPE,
    POOL_FORMAT_ERR_MEM,
}
class "struct format" << (S,#40A0F0) >>{
    char* name;
    int format_num;
    struct format_item* items;
    char* pos; // position
}
class "struct format_item" << (S,#40A0F0) >>{
    char* remark;
    int key_code;
    int params_num;
    struct format_param* params;
    char* pos; // position
}
class "struct format_param" << (S,#40A0F0) >>{
    char* type;
    char* name;
}
pool_format --* "struct format"
pool_format --* "enum state"
"struct format" --* "struct format_item"
"struct format_item" --* "struct format_param"
@enduml
/************************************************************************************************************************
@startuml
class unpack << (M,#80F0F0) >>{
    struct parsed_data* unpack(uint8_t* data, int len);
    void unpack_free(struct parsed_data* data);
}
class "struct parsed_data" << (S,#40A0F0) >>{
    struct parsed_item* item;
    struct parsed_data* next;
}
class "struct parsed_item" << (S,#40A0F0) >>{
    const struct param* param_type;
    int out_priority;
    int indent;
    const uint8_t* data;
    int bit_width;
    char* title;
    char** lines;
    int line_num;
}
unpack --* "struct parsed_data"
"struct parsed_data" --* "struct parsed_item"
@enduml
/************************************************************************************************************************/
/************************************************************************************************************************/