#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include "bt_usb.h"

#define TEST_NUM 0x1000

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
uint32_t pending_flag;

void eb_h4_recv(uint8_t *data, int len)
{
    pthread_mutex_lock(&lock);
    pending_flag = 0;
    pthread_mutex_unlock(&lock);
}

int main(void)
{
    srand(0);
    usb_hci_init(3, eb_h4_recv);
    int i;
    struct {
        uint8_t len;
        uint8_t data[255];
    } test_case[] = {
        { 4,  {0x01, 0x03, 0x0c, 0x00}, },
        { 4,  {0x01, 0x03, 0x0c, 0x00}, },
        { 4,  {0x01, 0x09, 0x10, 0x00, }, },
        { 12, {0x01, 0x01, 0x0c, 0x08, 0x90, 0x88, 0x00, 0x02, 0x00, 0x80, 0x00, 0x20}, },
        { 12, {0x01, 0x63, 0x0c, 0x08, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00}, },
        { 4,  {0x01, 0x01, 0x10, 0x00}, },
        { 4,  {0x01, 0x03, 0x20, 0x00}, },
        { 4,  {0x01, 0x02, 0x20, 0x00}, },
    };
    for(i=0;i<TEST_NUM;i++){
        int flag;
        do{
            pthread_mutex_lock(&lock);
            flag = pending_flag;
            pthread_mutex_unlock(&lock);
        }while(flag);
        int idx = rand() % sizeof(test_case)/sizeof(test_case[0]);
        pthread_mutex_lock(&lock);
        pending_flag = 1;
        pthread_mutex_unlock(&lock);
        usb_hci_send(test_case[idx].data, test_case[idx].len);
    }
    printf("PASS!\n");
}
