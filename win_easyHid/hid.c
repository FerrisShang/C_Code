#include <stdint.h>
#include "win_hid_map.c"

uint8_t hid_report_map[] = {
	0x05,0x01,0x09,0x06,0xa1,0x01,0x85,0x01,0x05,0x07,0x19,0xe0,0x29,0xe7,0x15,0x00,
	0x25,0x01,0x75,0x01,0x95,0x08,0x81,0x02,0x95,0x01,0x75,0x08,0x81,0x01,0x95,0x06,
	0x75,0x08,0x15,0x00,0x25,0x65,0x05,0x07,0x19,0x00,0x29,0x65,0x81,0x00,0x09,0x05,
	0x15,0x00,0x26,0xff,0x00,0x75,0x08,0x95,0x02,0xb1,0x02,0xc0,0x05,0x01,0x09,0x02,
	0xa1,0x01,0x85,0x02,0x09,0x01,0xa1,0x00,0x05,0x09,0x19,0x01,0x29,0x08,0x15,0x00,
	0x25,0x01,0x75,0x01,0x95,0x08,0x81,0x02,0x05,0x01,0x16,0x08,0xff,0x26,0xff,0x00,
	0x75,0x10,0x95,0x02,0x09,0x30,0x09,0x31,0x81,0x06,0x15,0x81,0x25,0x7f,0x75,0x08,
	0x95,0x01,0x09,0x38,0x81,0x06,0xc0,0xc0
};

const uuid16_t serv_hid    = {0x12, 0x18};
const uuid16_t rep_map     = {0x4B, 0x2A};
const uuid16_t report      = {0x4D, 0x2A};
const uuid16_t hid_info    = {0x4A, 0x2A};
const uuid16_t serv_gap    = {0x00, 0x18};
const uuid16_t gap_name    = {0x00, 0x2A};
const uuid16_t gap_appe    = {0x01, 0x2A};
const uuid16_t serv_gatt   = {0x01, 0x18};
const uuid16_t gatt_change = {0x05, 0x2A};


#define HANDLE_REPORT_MAP       3
#define HANDLE_REPORT1_VALUE    5
#define HANDLE_REPORT1_REF      6
#define HANDLE_REPORT1_CFG      7
#define HANDLE_REPORT2_VALUE    9
#define HANDLE_REPORT2_REF      10
#define HANDLE_REPORT2_CFG      11
#define HANDLE_HID_INFO         13

#define HANDLE_GAP_NAME         16
#define HANDLE_GAP_APPE         18

const eb_att_db_t att_db[] = {
	{{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv_hid,    1,0,  1,0,0,0,0, 0,0,},

	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&rep_map},                  NULL,                0,0,  1,0,0,0,0, 0,0,},

	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&report},                   NULL,                0,0,  1,0,0,1,0, 1,0,},
	{{&ATT_DESC_REPORT_REF},      NULL,                0,0,  1,0,0,0,0, 1,0,},
	{{&ATT_DESC_CLIENT_CHAR_CFG}, NULL,                0,0,  1,1,0,0,0, 0,1,},

	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&report},                   NULL,                0,0,  1,0,0,1,0, 1,0,},
	{{&ATT_DESC_REPORT_REF},      NULL,                0,0,  1,0,0,0,0, 1,0,},
	{{&ATT_DESC_CLIENT_CHAR_CFG}, NULL,                0,0,  1,1,0,0,0, 0,1,},

	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&hid_info},                 NULL,                0,0,  1,0,0,0,0, 1,0,},


	{{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv_gap,    1,0,  1,0,0,0,0, 0,0,},
	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&gap_name},                 NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&gap_appe},                 NULL,                0,0,  1,0,0,0,0, 0,0,},

	{{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv_gatt,   1,0,  1,0,0,0,0, 0,0,},
	{{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
	{{&gatt_change},              NULL,                0,0,  1,0,0,0,0, 0,0,},
};

