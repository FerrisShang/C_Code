#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "csv_read.h"
#include "utils.h"

#define LINE_MAX 1024
#define c_calloc(s, n) util_calloc(s, n)
#define c_free(p) util_free(p)
#define csv_debug printf

int csv_read(char* filename, struct csv_data* data)
{
    struct csv_data m_data = {{0}};
    FILE* fp;
    char line_buf[LINE_MAX];
    memset(data, 0, sizeof(struct csv_data));
    fp = fopen(filename, "r");
    if (!fp) {
        return CSV_FILE_NOT_FOUND;
    }
    m_data.line_num = 0;
    assert(strlen(filename) < 32);
    strcpy(m_data.filename, filename);
    // malloc csv buffer
    int n = 0;
    while (!feof(fp)) {
        fgets(line_buf, LINE_MAX, fp);
        n++;
    }
    m_data.lines = (struct csv_line*)c_calloc(sizeof(struct csv_line), n);
    // parsing csv
    fseek(fp, 0, SEEK_SET);
    while (!feof(fp)) {
        fgets(line_buf, LINE_MAX, fp);
        char* p = strip(line_buf, NULL);
        struct csv_line* lines = &m_data.lines[m_data.line_num];
        lines->col_num = 0;
        if (strlen(p) == 0) {
            continue;
        }

        // malloc lines buffer
        lines->cells = (struct csv_cell*)c_calloc(sizeof(struct csv_cell), strcnt(p, ",") + 1);
        lines->col_num = 0;
        // parsing line
        while (p) {
            struct csv_cell* cell = &lines->cells[lines->col_num];
            int len;
            char* end = strstr(p, ",");
            char* np = end && *end ? end + 1 : end;
            if (end) *end = '\0';
            p = strip(p, &len);
            cell->x = lines->col_num;
            cell->y = m_data.line_num;
            cell->text = strdup(p);
            lines->col_num++;
            p = np;
        }
        if (lines->col_num) {
            m_data.line_num++;
        }
    }
    *data = m_data;
    fclose(fp);
    return CSV_SUCCESS;
}

void csv_dump(struct csv_data* data)
{
    int i, j;
    for (i = 0; i < data->line_num; i++) {
        struct csv_line* lines = &data->lines[i];
        for (j = 0; j < lines->col_num; j++) {
            struct csv_cell* cell = &lines->cells[j];
            csv_debug("%s(%c%d)\t", cell->text, cell->x + 'A', cell->y + 1);
        }
        csv_debug("\n");
    }
}
void csv_free(struct csv_data* data)
{
    if (data->lines) {
        for (int i = 0; i < data->line_num; i++) {
            if (data->lines[i].cells) {
                for (int j = 0; j < data->lines[i].col_num; j++) {
                    free(data->lines[i].cells[j].text);
                    data->lines[i].cells[j].text = NULL;
                }
                c_free(data->lines[i].cells);
            }
        }
        c_free(data->lines);
        data->lines = NULL;
    }
}
void csv_output_c(struct csv_data* data)
{
    char buf[4096];
    int i, j, line_num;
    char *fn = strdup(data->filename);
    fn[strlen(fn)-4] = '\0';
    sprintf(buf, "#include \"csv_read.h\"\n");
    csv_debug("%s", buf);
    for (i = 0; i < data->line_num; i++) {
        struct csv_line* lines = &data->lines[i];
        int col_num = 0;
        for (j = lines->col_num; j>0; j--) {
            struct csv_cell* cell = &lines->cells[j-1];
            if(strlen(cell->text) > 0){ col_num = j; }
        }
        if(col_num == 0){ continue; }
        sprintf(buf, "const struct csv_cell line_%s_%d[] = {\n", fn, i);
        csv_debug("%s", buf);
        for (j = 0; j < lines->col_num; j++) {
            struct csv_cell* cell = &lines->cells[j];
            sprintf(buf, "    { %d, %d, \"%s\" },\n", cell->x, cell->y, cell->text);
            csv_debug("%s", buf);
        }
        sprintf(buf, "};\n");
        csv_debug("%s", buf);
    }
    line_num = i-1;
    sprintf(buf, "const struct csv_line lines_%s[] = {\n", fn);
    csv_debug("%s", buf);
    for (i = 0; i < line_num; i++) {
        struct csv_line* lines = &data->lines[i];
        int col_num = 0;
        for (j = lines->col_num; j>0; j--) {
            struct csv_cell* cell = &lines->cells[j-1];
            if(strlen(cell->text) > 0){ col_num = j; }
        }
        if(col_num == 0){ continue; }
        sprintf(buf, "    { sizeof(line_%s_%d)/sizeof(line_%s_%d[0]), (struct csv_cell*)line_%s_%d },\n",
                fn, i, fn, i, fn, i);
        csv_debug("%s", buf);
    };
    sprintf(buf, "};\n");
    csv_debug("%s", buf);

    sprintf(buf, "struct csv_data csv_data_%s = {\n", fn);
    csv_debug("%s", buf);
    sprintf(buf, "    \"%s\", %d, (struct csv_line*)lines_%s\n", data->filename, line_num, fn);
    csv_debug("%s", buf);
    sprintf(buf, "};\n");
    csv_debug("%s", buf);

    free(fn);
}

