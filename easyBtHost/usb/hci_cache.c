#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define BUF_SIZE (256)
#define BUF_NUM  (1<<5)

static uint8_t hci_buf[BUF_NUM][BUF_SIZE];
static pthread_mutex_t lock;
static int fr, ra;

void hci_buf_in(uint8_t *data, int len)
{
    pthread_mutex_lock(&lock);
    int i;
    assert(len <= BUF_SIZE);
    ra=(ra+1)&(BUF_NUM-1);
    assert(fr != ra); // full
    for(i=0;i<len;i++){ hci_buf[ra][0] = len; memcpy(&hci_buf[ra][1], data, len); }
    pthread_mutex_unlock(&lock);
}

uint8_t hci_buf_cnt(void)
{
    pthread_mutex_lock(&lock);
    uint8_t res = (ra - fr) & (BUF_NUM-1);
    pthread_mutex_unlock(&lock);
    return res;
}

uint8_t* hci_buf_get(uint8_t* len)
{
    pthread_mutex_lock(&lock);
    assert(fr != ra); // empty
    fr=(fr+1)&(BUF_NUM-1);
    *len = hci_buf[fr][0];
    uint8_t *data = &hci_buf[fr][1];
    pthread_mutex_unlock(&lock);
    return data;
}

