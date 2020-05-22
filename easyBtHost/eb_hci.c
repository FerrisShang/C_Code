#include <string.h>
#include "eb_hci.h"


void eb_hci_init(void)
{
    ;
}

static void em_hci_encrypted_handler(uint8_t *encrypted);

void eb_smp_encrpyt_change(void);
void eb_gap_set_encrypted(uint16_t con_hdl, bool encrypted);
void eb_gap_connected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type);
void eb_smp_connected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type);
bool eb_smp_get_ltk(void);
void eb_gap_disconnected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type);
void eb_smp_disconnected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type);

void eb_hci_handler(uint8_t *data, uint16_t len)
{
    switch(data[0]){
        case 2: // ACL data
            assert(len >= 9); // 0x02 + HANDLE + ACL_LEN + L2CAP_LEN + CID
            switch(data[7]){
                case 4: // ATT
                    eb_att_handler(data, len);
                    break;
                case 5: // LE signal
                    eb_l2cap_handler(data, len);
                    break;
                case 6: // SMP
                    eb_smp_handler(data, len);
                    break;
                default:
                    // Unknown/Dynamic CID
                    { // response unknows CID
                        uint16_t conn_hd = (data[1] + (data[2]<<8)) & 0x0FFF;
                        uint8_t cmd[9+10] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x0e, 0x00, 0x0a, 0x00, 0x05, 0x00, 0x01, 0x00, 0x06, 0x00, 0x02, 0x00,
                                            data[7], data[8], 0x00, 0x00}; // Invalid CID
                        eb_h4_send(cmd, sizeof(cmd));
                    }
                    break;
            }
            break;
        case 4:{ // HCI Event
            switch(data[1]){ // Event Code
                case 0x08:{ // Encryption Change Event
                    eb_event_t evt = { EB_EVT_GAP_ENCRYPTED };
                    evt.gap.encrypt_change.handle = data[4] + (data[5]<<8);
                    evt.gap.encrypt_change.status = data[6];
                    eb_smp_encrpyt_change();
                    eb_gap_set_encrypted(evt.gap.encrypt_change.handle, true);
                    eb_event(&evt);
                    break;}
                case 0x0E:{ // Command Complete
                    uint16_t opcode = data[4] + (data[5]<<8);
                    switch(opcode){
                        case 0x2017:{ // LE Encrypt
                            em_hci_encrypted_handler(&data[7]);
                        break;}
                        case 0x0C03:{ // Reset
                            eb_event_t evt = { EB_EVT_GAP_RESET };
                            eb_event(&evt);
                        break;}
                    }
                    break;}
                case 0x0F:{ // Command Status
                    
                    break;}
                case 0x3E:{ // LE Meta Event
                    assert(len == 3+data[2]);
                    uint8_t len = data[2];
                    uint8_t sub_event = data[3];
                    switch(sub_event){
                        case 0x01:{ // LE Connection Complete event
                            eb_event_t evt = { EB_EVT_GAP_CONNECTED };
                            evt.gap.connected.status = data[4];
                            evt.gap.connected.handle = data[5] + (data[6]<<8);
                            evt.gap.connected.role = data[7];
                            evt.gap.connected.peer_addr_type = data[8];
                            memcpy(&evt.gap.connected.peer_addr[0], &data[9], sizeof(bdaddr_t));
                            evt.gap.connected.interval = data[15] + (data[16]<<8);
                            evt.gap.connected.latency = data[17] + (data[18]<<8);
                            evt.gap.connected.timeout = data[19] + (data[20]<<8);
                            evt.gap.connected.sca = data[21];
                            eb_gap_connected_handler(evt.gap.connected.handle, evt.gap.connected.peer_addr, data[8]);
                            eb_smp_connected_handler(evt.gap.connected.handle, evt.gap.connected.peer_addr, data[8]);
                            eb_event(&evt);
                            break;}
                        case 0x02:{ // LE Advertising Report event
                            if(data[4] != 1){ break; } // Number Reports
                            eb_event_t evt = { EB_EVT_GAP_ADV_REPORT };
                            evt.gap.adv_report.type = data[5];
                            evt.gap.adv_report.addr_type = data[6];
                            memcpy(evt.gap.adv_report.addr, &data[7], 6);
                            evt.gap.adv_report.length = data[8];
                            evt.gap.adv_report.data = &data[9];
                            evt.gap.adv_report.rssi = data[2 + data[2]];
                            eb_event(&evt);
                            break;}
                        case 0x05:{ // LE Long Term Key Request Event
                            if(!eb_smp_get_ltk()){
                                uint8_t ltk[16] = {0};
                                eb_event_t evt = { EB_EVT_GAP_LTK_REQUEST };
                                evt.gap.ltk_request.handle = data[4] + (data[5]<<8);
                                evt.gap.ltk_request.random = &data[6];
                                evt.gap.ltk_request.ediv = &data[14];
                                evt.gap.ltk_request.ltk_found = false;
                                evt.gap.ltk_request.ltk = ltk;
                                eb_event(&evt);
                                if(evt.gap.ltk_request.ltk_found){
                                    uint8_t cmd[4+18] = {0x01, 0x1A, 0x20, 0x12};
                                    cmd[4] = data[4]; cmd[5] = data[5];
                                    memcpy(&cmd[6], ltk, 16);
                                    eb_h4_send(cmd, sizeof(cmd));
                                }else{
                                    uint8_t cmd[4+2] = {0x01, 0x1B, 0x20, 0x12};
                                    cmd[4] = data[4]; cmd[5] = data[5];
                                    eb_h4_send(cmd, sizeof(cmd));
                                }
                            }
                            break;}
                        
                    }
                    break;}
                case 0x05:{ // Disconnection Complete Event
                    eb_event_t evt = { EB_EVT_GAP_DISCONNECTED };
                    evt.gap.disconnected.status = data[3];
                    evt.gap.disconnected.handle = data[4] + (data[5]<<8);
                    evt.gap.disconnected.reason = data[6];
                    eb_gap_disconnected_handler(evt.gap.connected.handle, evt.gap.connected.peer_addr, data[8]);
                    eb_smp_disconnected_handler(evt.gap.connected.handle, evt.gap.connected.peer_addr, data[8]);
                    eb_event(&evt);
                    break;}
            }
            break;}
        default:
            assert(0);
    }
}

void (*m_hci_encrypt_cb)(uint8_t *encrypted);
bool em_hci_encrypt(uint8_t *plaintext, uint8_t *key, void (*encrypt_cb)(uint8_t *))
{
    if(m_hci_encrypt_cb){
        return false;
    }
    m_hci_encrypt_cb = encrypt_cb;
    uint8_t cmd[4+32] = {0x01, 0x17, 0x20, 0x20};
    memcpy(&cmd[4], key, 16);
    memcpy(&cmd[4+16], plaintext, 16);
    eb_h4_send(cmd, sizeof(cmd));
    return true;
}

static void em_hci_encrypted_handler(uint8_t *encrypted)
{
    if(m_hci_encrypt_cb){
        void (*callback)(uint8_t *) = m_hci_encrypt_cb;
        m_hci_encrypt_cb = NULL;
        callback(encrypted);
    }
}

