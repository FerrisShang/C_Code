#ifndef __BTSNOOP_REC_H__
#define __BTSNOOP_REC_H__

#include <stdio.h>
#define DATA_DIR_SEND 0
#define DATA_DIR_RECV 1

FILE *create_btsnoop_rec(char *path);
void close_btsnoop_rec(FILE *fp);
void record_btsnoop(FILE *fp, char *hci_data, int data_len, char data_dir);

#endif /* __BTSNOOP_REC_H__ */
