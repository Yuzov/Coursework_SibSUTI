#include "md5.h"
#include "strings.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define PTR_SIZE 256
#define MAX_PATH 260
#define NAME_SIZE 255
#define TYPE_SIZE 5

struct file_record {
    unsigned int id;
    char name[NAME_SIZE];
    unsigned int parent_id;
    char type[TYPE_SIZE];
    unsigned char hash[16];
};
typedef struct file_record Record;

int save_info(char* path, char* database)
{
    struct md5_ctx md5ctx;
    unsigned char md5_result[16];
    // md5_result[0] = '\0';
    for (int i = 0; i < 16; i++) {
        md5_result[i] = '0';
    }
    unsigned size;

    Record record;
    for (int i = 0; i < NAME_SIZE; i++) {
        record.name[i] = '\0';
    }
    // record.hash[0] = '\0';
    bool file = true, dir = true;
    char* ptr[PTR_SIZE];
    int nesting_cnt;
    FILE* input;
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
        scopy("file", record.type);
        printf("IT'S A FILE!\n");
    } else {
        if (*(path + slen(path) - 1) == '/')
            *(path + slen(path) - 1) = '\0';
        scopy("dir", record.type);
        printf("IT'S A DIRECTORY!\n");
    }
    // fclose(input);

    //Запись в файл
    FILE* data;
    if ((data = fopen("database.bin", "r+b")) == NULL) {
        data = fopen("database.bin", "w+b");
        rewind(data);
    }

    //Проверка

    // fwrite(&record, 1, sizeof(record), data);
    nesting_cnt = stok(path, '/', ptr);
    char dir_name[NAME_SIZE];
    scopy(ptr[nesting_cnt - 1] + 1, dir_name);
    scopy(ptr[nesting_cnt - 1] + 1, record.name);
    suntok(path, '/', ptr, nesting_cnt);

    //Проверка, что данная директория уже добавлена
    fseek(data, 0, SEEK_END);
    record.id = 1;
    unsigned int pos = ftell(data);
    if (pos > 0) {
        fseek(data, 0, SEEK_SET);
        FILE* f;
        f = fopen("database.bin", "r+b");
        while (!feof(f)) {
            Record buf;
            if (fread(&buf, 1, sizeof(buf), f) > 0) {
                if ((scmp(buf.name, record.name) == 0)
                    && (buf.parent_id == record.parent_id)
                    && (scmp(buf.type, record.type) == 0)) {
                    printf("This directory is already recorded\n");
                    return 0;
                }
            }
        }
    }
    //Запись данной директории
    record.id = 1;
    record.parent_id = 0;
    fwrite(&record, 1, sizeof(record), data);
    fclose(input);

    if (dir == true) {
        DIR* dir = opendir(path);
        if (dir == NULL) {
            return -5;
        }
        struct dirent* entity;
        entity = readdir(dir);
        while (entity != NULL) {
            // unsigned int id = 1;
            if (entity->d_type == DT_REG) {
                record.id++;
                record.parent_id = 1;
                scopy(entity->d_name, record.name);

                scopy("file", record.type);

                // MD5
                int path_len = slen(path);
                scat(path, "/");
                scat(path, record.name);
                input = fopen(path, "rb");

                md5_init(&md5ctx);
                fseek(input, 0, SEEK_END);
                size = ftell(input);
                fseek(input, 0, SEEK_SET);
                unsigned char* msg = malloc(sizeof(unsigned char) * size);
                fread(msg, size, 1, input);
                md5_update(&md5ctx, msg, size);
                md5_final(&md5ctx, record.hash);

                fclose(input);

                *(path + path_len) = '\0';

                fwrite(&record, 1, sizeof(record), data);

                printf("%hhd %s\n", entity->d_type, entity->d_name);
            }
            entity = readdir(dir);
        }
        closedir(dir);
    }
    return 0;
}
int check_integrity(char* path)
{
    struct md5_ctx md5ctx;
    Record record;
    record.id = 1;
    record.parent_id = 0;
    DIR* dir = opendir(path);
    char* ptr[PTR_SIZE];
    if (dir == NULL) {
        return -5; // Был удален
    }
    struct dirent* entity;
    // entity = readdir(dir);

    int nesting_cnt = stok(path, '/', ptr);
    char dir_name[NAME_SIZE];
    scopy(ptr[nesting_cnt - 1] + 1, dir_name);
    scopy(ptr[nesting_cnt - 1] + 1, record.name);
    suntok(path, '/', ptr, nesting_cnt);

    scopy("dir", record.type);
    FILE* data;
    if ((data = fopen("database.bin", "rb")) == NULL) {
        return -6; //БД отсутствует
    }
    fseek(data, 0, SEEK_END);
    unsigned int pos = ftell(data);
    if (pos > 0) {
        fseek(data, 0, SEEK_SET);

        FILE* f;
        FILE* input;
        f = fopen("database.bin", "r+b");
        Record buf;
        while (!feof(f)) {
            if (fread(&buf, 1, sizeof(buf), f) > 0) {
                if ((scmp(buf.name, record.name) == 0) && (buf.id == record.id)
                    && (buf.parent_id == record.parent_id)) {
                    printf("This directory exists in database\n");
                    break;
                }
            }
        }
        while (entity != NULL) {
            while (entity->d_type != DT_REG) {
                entity = readdir(dir);
            }
            if (entity == NULL)
                break;
            bool hash_match = true;
            if (entity->d_type == DT_REG) {
                record.id++;
                record.parent_id = 1;
                scopy(entity->d_name, record.name);

                scopy("file", record.type);
            }
            if (fread(&buf, 1, sizeof(buf), f) > 0) {
                int path_len = slen(path);
                scat(path, "/");
                scat(path, buf.name);
                input = fopen(path, "rb");

                if (input == NULL) {
                    printf("File %s was deleted\n", buf.name);
                    entity = readdir(dir);
                    *(path + path_len) = '\0';
                } else if (
                        (scmp(entity->d_name, record.name) != 0)
                        || (buf.id != record.id)
                        || (buf.parent_id != record.parent_id)) {
                    printf("File %s was deleted\n", record.name);
                    entity = readdir(dir);
                } else {
                    md5_init(&md5ctx);
                    fseek(input, 0, SEEK_END);
                    int size = ftell(input);
                    fseek(input, 0, SEEK_SET);
                    unsigned char* msg = malloc(sizeof(unsigned char) * size);
                    fread(msg, size, 1, input);
                    md5_update(&md5ctx, msg, size);
                    md5_final(&md5ctx, record.hash);

                    fclose(input);

                    *(path + path_len) = '\0';

                    for (int i = 0; i < 16; i++) {
                        if (buf.hash[i] != record.hash[i]) {
                            hash_match = false;
                            break;
                        }
                    }
                    if ((scmp(entity->d_name, record.name) == 0)
                        && (buf.id == record.id)
                        && (buf.parent_id == record.parent_id)
                        && (hash_match == true)) {
                        entity = readdir(dir);
                    } else {
                        entity = readdir(dir);
                        printf("File %s was changed\n", record.name);
                    }
                }
            }
        }

    } else
        return -7;
    //БД пустая
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
        // return 0;
    }
    // if ((scmp(argv[1], "-c")) && (scmp(argv[2], "-f"))
    //    && (scmp(argv[3], "database"))) {
    check_integrity(argv[4]);
    return 0;
    //}
    return 0;
}
