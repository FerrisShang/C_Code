#ifndef __EB_ATT_H__
#define __EB_ATT_H__

#include "eb_config.h"

typedef uint8_t uuid16_t[2];
typedef uint8_t uuid128_t[16];
typedef struct eb_att_db{
    union{
        const uuid16_t *uuid16;
        const uuid128_t *uuid128;
    };
    uint8_t *value;
    uint8_t is_service     :1;
    uint8_t uuid_len       :1;
    uint8_t prop_read      :1;
    uint8_t prop_write     :1;
    uint8_t prop_write_cmd :1;
    uint8_t prop_ntf       :1;
    uint8_t prop_ind       :1;
    uint8_t perm_read      :1;
    uint8_t perm_write     :1;
}eb_att_db_t;
void eb_att_init(void);
void eb_att_handler(uint8_t *data, uint16_t len);
void eb_att_set_service(const eb_att_db_t* db, uint16_t len);
uint16_t eb_att_get_service(const eb_att_db_t **db);
void eb_att_error_response(uint16_t conn_hd, uint16_t att_hd, uint8_t opcode, uint8_t err_code);

extern const uuid16_t ATT_DECL_PRIMARY_SERVICE;
extern const uuid16_t ATT_DECL_CHARACTERISTIC;
extern const uuid16_t ATT_DESC_CHAR_USER_DESCRIPTION;
extern const uuid16_t ATT_DESC_CLIENT_CHAR_CFG;
extern const uuid16_t ATT_DESC_REPORT_REF;

#endif /* __EB_ATT_H__ */