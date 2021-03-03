#ifndef __CSV_READ_H__
#define __CSV_READ_H__

#include <stdint.h>

struct csv_cell {
    int x;
    int y;
    char* text;
};

struct csv_line {
    int col_num;
    struct csv_cell* cells;
};

struct csv_data {
    char filename[32];
    int line_num;
    struct csv_line* lines;
};

enum {
    CSV_SUCCESS,
    CSV_FILE_NOT_FOUND,
};

int csv_read(char* filename, struct csv_data* data);
void csv_free(struct csv_data* data);
void csv_dump(struct csv_data* data);
void csv_output_c(struct csv_data* data);


#endif /* __CSV_READ_H__ */
