#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include "hci_cache.h"

#define BUF_SIZE (1<<10)
#define TEST_NUM (1<<20)

int test_len_rec[TEST_NUM];
void* th1_cb(void* p)
{
    uint32_t i;
    uint8_t buf[BUF_SIZE];
    for(i=0;i<TEST_NUM;i++){
        int len = rand() & (BUF_SIZE-1);
        len = len<BUF_SIZE-4?len:BUF_SIZE-4;
        memset(buf, (uint8_t)i, len);
        test_len_rec[i] = len;
        while(hci_buf_is_full());
        //printf(">>>> %05d %d\n", i, len);
        hci_buf_in(buf, len);
    }
    return NULL;
}

void* th2_cb(void* p)
{
    uint32_t i, j, len;
    uint8_t *buf;
    for(i=0;i<TEST_NUM;i++){
        while(!hci_buf_cnt());
        buf = hci_buf_get(&len);
        //printf("<--- %05d %d\n", i, len);
        for(j=0;j<len;j++){
            if(buf[j] != (uint8_t)i){
                printf("%d vs %d(%d)\n", buf[j], i, j);
                assert(0);
            }
        }
        if(len != test_len_rec[i]) {
            printf("%d vs %d\n", len, test_len_rec[i]);
            assert(len == test_len_rec[i]);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    srand(0);
    pthread_t th1, th2;
    pthread_create(&th1, NULL, th1_cb, NULL);
    pthread_create(&th2, NULL, th2_cb, NULL);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    printf("PASS!\n");
    return 0;
}

