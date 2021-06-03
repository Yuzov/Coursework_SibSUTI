#include "md5.h"
#include "strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define PTR_SIZE 256
#define MAX_PATH 260
#define NAME_SIZE 255
#define TYPE_SIZE 4

struct file_record {
    char* name;
    char* type;
    unsigned int parent_id;
    unsigned char hash[16];
};
typedef struct file_record Record;

int save_info(char* path, char* database)
{
    struct md5_ctx md5ctx;
    unsigned char md5_result[16];
    unsigned size;

    Record record;
    bool file = true, dir = true;
    char* ptr[PTR_SIZE];
    int nesting_cnt;
    FILE* input;
    FILE* data;
    if (slen(path) > MAX_PATH) {
        return -1;
    }
    input = fopen(path, "r+");
    if (input == NULL) {
        file = false;
    }
    input = fopen(path, "r");
    if (input == NULL) {
        dir = false;
    }
    if ((file == false) && (dir == false)) {
        printf("This file or directory doesn't exist\n");
        return -4;
    } else if (file == true) {
        record.type = "file";
        printf("IT'S A FILE!\n");
    } else {
        if (*(path + slen(path) - 1) == '/')
            *(path + slen(path) - 1) = '\0';
        record.type = "dir";
        printf("IT'S A DIRECTORY!\n");
    }
    nesting_cnt = stok(path, '/', ptr);
    record.name = ptr[nesting_cnt];
    record.parent_id = nesting_cnt - 1;
    printf("Nesting counter = %d\n", nesting_cnt);
    if (file == true) {
        md5_init(&md5ctx);
        // size = fread(buf, 1, );
        fseek(input, 0, SEEK_END);
        size = ftell(input);
        fseek(input, 0, SEEK_SET);
        // unsigned char* msg = malloc(sizeof(unsigned char) * size);
        // fread(msg, size, 1, input);
        md5_update(&md5ctx, input, size);
        md5_final(&md5ctx, md5_result);
    }
    return 0;
}
int check_integrity()
{
    return 0;
}

int main(int argc, char* argv[])
{
    char* database = NULL;
    printf("%d\n", argc);
    if (argc != 5) {
        printf("Usage:\n");
        printf("./integrctrl -s -f database <path to file or directory>\n");
        printf("./integrctrl -c -f database <path to file or directory>\n");
        return -1;
    }
    printf("%s\n %s\n %s\n %s\n %s\n",
           argv[0],
           argv[1],
           argv[2],
           argv[3],
           argv[4]);

    if (((scmp(argv[1], "-s") != 0) && (scmp(argv[1], "-c") != 0))) {
        printf("Expected -s or -c as first argument\n");
        return -2;
    }
    if (scmp(argv[2], "-f") != 0) {
        printf("Expected -f as second argument\n");
        return -3;
    }

    if ((scmp(argv[1], "-s") == 0) && (scmp(argv[2], "-f") == 0)
        && (scmp(argv[3], "database") == 0)) {
        // printf("Came\n");
        save_info(argv[4], database);
        return 0;
    }
    if ((scmp(argv[1], "-c")) && (scmp(argv[2], "-f"))
        && (scmp(argv[3], "database"))) {
        check_integrity();
        return 0;
    }
    return 0;
}
