#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define BUF_SIZE (1<<10)
#define BUF_NUM  (1<<8)

__attribute__((aligned(4))) static uint8_t hci_buf[BUF_NUM][BUF_SIZE];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int fr, ra;

void hci_buf_in(uint8_t *data, uint32_t len)
{
    pthread_mutex_lock(&lock);
    assert(len <= BUF_SIZE-sizeof(uint32_t));
    ra=(ra+1)&(BUF_NUM-1);
    assert(fr != ra); // full
    void *p = hci_buf[ra];
    *(uint32_t*)p = len;
    memcpy(&hci_buf[ra][sizeof(uint32_t)], data, len);
    pthread_mutex_unlock(&lock);
}

uint32_t hci_buf_cnt(void)
{
    pthread_mutex_lock(&lock);
    uint32_t res = (ra - fr) & (BUF_NUM-1);
    pthread_mutex_unlock(&lock);
    return res;
}

uint8_t* hci_buf_get(uint32_t* len)
{
    static uint8_t data[BUF_SIZE];
    pthread_mutex_lock(&lock);
    assert(fr != ra); // empty
    fr=(fr+1)&(BUF_NUM-1);
    void *p = hci_buf[fr];
    *len = *(uint32_t*)p;
    memcpy(data, &hci_buf[fr][sizeof(uint32_t)], *len);
    pthread_mutex_unlock(&lock);
    return data;
}

int hci_buf_is_full(void)
{
    pthread_mutex_lock(&lock);
    uint32_t res = (fr==((ra+1)&(BUF_NUM-1)));
    pthread_mutex_unlock(&lock);
    return res;
}
