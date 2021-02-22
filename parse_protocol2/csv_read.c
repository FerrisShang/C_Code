#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_read.h"
#include "utils.h"

#define LINE_MAX 1024
#define csv_calloc(s, n) calloc(s, n)
#define csv_free(p) free(p)
#define csv_debug printf

static struct csv_data m_data;

int csv_read(char* filename, struct csv_data* data)
{
    FILE* fp;
    char line_buf[LINE_MAX];
    fp = fopen(filename, "r");
    if (!fp) {
        return CSV_FILE_NOT_FOUND;
    }
    *data = m_data;
    m_data.line_num = 0;
    // malloc csv buffer
    int n = 0;
    while (!feof(fp)) {
        fgets(line_buf, LINE_MAX, fp);
        n++;
    }
    if (m_data.lines) {
        csv_free(m_data.lines);
    }
    m_data.lines = csv_calloc(sizeof(struct csv_line), n);
    // parsing csv
    fseek(fp, 0, SEEK_SET);
    while (!feof(fp)) {
        fgets(line_buf, LINE_MAX, fp);
        char* p = strip(line_buf, NULL);
        struct csv_line* lines = &m_data.lines[m_data.line_num];
        lines->col_num = 0;

        // malloc lines buffer
        if (lines->cells) {
            csv_free(lines->cells);
        }
        lines->cells = csv_calloc(sizeof(struct csv_cell), strcnt(p, ",") + 1);
        lines->col_num = 0;
        // parsing line
        while (p) {
            struct csv_cell* cell = &lines->cells[lines->col_num];
            int len;
            char* end = strstr(p, ",");
            char* np = end && *end ? end + 1 : end;
            if (end) *end = '\0';
            p = strip(p, &len);
            char* str = csv_calloc(len + 1, 1);
            strcpy(str, p);
            cell->x = lines->col_num;
            cell->y = m_data.line_num;
            cell->text = str;
            lines->col_num++;
            p = np;
        }
        if (lines->col_num) {
            m_data.line_num++;
        }
    }
    return CSV_SUCCESS;
}

void csv_dump(void)
{
    int i, j;
    for (i = 0; i < m_data.line_num; i++) {
        struct csv_line* lines = &m_data.lines[i];
        for (j = 0; j < lines->col_num; j++) {
            struct csv_cell* cell = &lines->cells[j];
            csv_debug("%s(%c%d)\t", cell->text, cell->x + 'A', cell->y + 1);
        }
        csv_debug("\n");
    }
}
