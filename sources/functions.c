#include "functions.h"
int check_dir(char* path, FILE* input, Record* record, bool* direct)
{
    bool file = true;
    if (slen(path) > MAX_PATH) {
        return -1;
    }
    input = fopen(path, "r+");
    if (input == NULL) {
        file = false;
    }
    input = fopen(path, "r");
    if (input == NULL) {
        *direct = false;
    }
    if ((file == false) && (*direct == false)) {
        printf("This file or directory doesn't exist\n");
        return -4;
    } else if (file == true) {
        scopy("file", record->type);
        printf("IT'S A FILE!\n");
    } else {
        if (*(path + slen(path) - 1) == '/')
            *(path + slen(path) - 1) = '\0';
        scopy("dir", record->type);
        printf("IT'S A DIRECTORY!\n");
    }
    fclose(input);
}

void get_hash(FILE* input, Record* record)
{
    int size;
    struct md5_ctx md5ctx;
    md5_init(&md5ctx);
    fseek(input, 0, SEEK_END);
    size = ftell(input);
    fseek(input, 0, SEEK_SET);
    unsigned char* msg = malloc(sizeof(unsigned char) * size);
    fread(msg, size, 1, input);
    md5_update(&md5ctx, msg, size);
    md5_final(&md5ctx, record->hash);
}

void concat_name(int* path_len, char* path, char* record_field)
{
    *path_len = slen(path);
    scat(path, "/");
    scat(path, record_field);
}