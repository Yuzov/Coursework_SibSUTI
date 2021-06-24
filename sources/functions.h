#define PTR_SIZE 256
#define MAX_PATH 260
#define NAME_SIZE 255
#define TYPE_SIZE 5
#include <stdbool.h>
#include <stdlib.h>
#include "md5.h"
#include "strings.h"
#include <dirent.h>
#include <stdio.h>

struct file_record {
    unsigned int id;
    char name[NAME_SIZE];
    unsigned int parent_id;
    char type[TYPE_SIZE];
    unsigned char hash[16];
};
typedef struct file_record Record;

int check_dir(char* path, FILE* input, Record *record, bool* direct);
void get_hash(FILE* input, Record* record);