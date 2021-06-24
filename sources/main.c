#include "functions.h"

int save_info(
        char* path,
        bool is_nested,
        unsigned int* id,
        unsigned int parent_id,
        FILE* data,
        DIR* dir)
{
    FILE* input;
    Record record;
    bool direct = true;
    char* ptr[PTR_SIZE];
    int nesting_cnt;

    for (int i = 0; i < NAME_SIZE; i++) {
        record.name[i] = '\0';
    }

    check_dir(path, input, &record, &direct);

    nesting_cnt = stok(path, '/', ptr);
    scopy(ptr[nesting_cnt - 1] + 1, record.name);
    suntok(path, '/', ptr, nesting_cnt);

    //Проверка, что данная директория уже добавлена
    fseek(data, 0, SEEK_END);
    unsigned int pos = ftell(data);
    if (pos > 0) {
        fseek(data, 0, SEEK_SET);
        FILE* f;
        f = fopen("database.bin", "r+b");
        while (!feof(f)) {
            Record buf;
            if (fread(&buf, 1, sizeof(buf), f) > 0) {
                *id = *id + 1;
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
    record.id = *id;
    record.parent_id = parent_id;
    fseek(data, 0, SEEK_END);
    fwrite(&record, 1, sizeof(record), data);

    if (direct == true) {
        dir = opendir(path);
        if (dir == NULL) {
            return -5;
        }
        struct dirent* entity;
        entity = readdir(dir);
        int path_len;

        while (entity != NULL) {
            if ((entity->d_type == DT_DIR) && (scmp(entity->d_name, ".") != 0)
                && (scmp(entity->d_name, "..") != 0) && (is_nested == true)) {
                path_len = slen(path);
                scat(path, "/");
                scat(path, entity->d_name);
                *id = *id + 1;
                save_info(path, is_nested, id, parent_id + 1, data, dir);
                *(path + path_len) = '\0';
            }

            if (entity->d_type == DT_REG) {
                record.id++;
                record.parent_id = parent_id + 1;
                scopy(entity->d_name, record.name);
                scopy("file", record.type);

                // MD5
                path_len = slen(path);
                scat(path, "/");
                scat(path, record.name);
                input = fopen(path, "rb");
                if (input != NULL) {
                    get_hash(input, &record);

                    *(path + path_len) = '\0';

                    fwrite(&record, 1, sizeof(record), data);

                    printf("%hhd %s\n", entity->d_type, entity->d_name);
                    *id = *id + 1;
                }
            }
            entity = readdir(dir);
        }
        closedir(dir);
    }
    return 0;
}
int check_integrity(
        char* path,
        unsigned int parent_id,
        FILE* data,
        FILE* f,
        FILE* input,
        Record buf,
        FILE* output)
{
    int path_len;
    Record record;
    record.id = 1;
    record.parent_id = parent_id;
    int file_counter = 0;

    if (*(path + slen(path) - 1) == '/')
        *(path + slen(path) - 1) = '\0';

    DIR* dir = opendir(path);
    char* ptr[PTR_SIZE];
    if (dir == NULL) {
        printf("Directory was DELETED");
        closedir(dir);
        return 0; // Был удален
    }
    closedir(dir);

    int nesting_cnt = stok(path, '/', ptr);
    scopy((ptr[nesting_cnt - 1] + 1), record.name);
    suntok(path, '/', ptr, nesting_cnt);

    scopy("dir", record.type);
    // FILE* data;
    if ((data = fopen("database.bin", "rb")) == NULL) {
        return -6; //БД отсутствует
    }
    fseek(data, 0, SEEK_END);
    unsigned int pos = ftell(data);
    if (pos > 0) {
        fseek(data, 0, SEEK_SET);
        f = fopen("database.bin", "r+b");
        while (!feof(f)) {
            if (fread(&buf, 1, sizeof(buf), f) > 0) {
                if ((scmp(buf.name, record.name)
                     == 0) //&& (buf.id == record.id)
                    && (buf.parent_id == record.parent_id)) {
                    printf("This directory exists in database\n");
                    break;
                }
            }
        }
        parent_id++;

        while (fread(&buf, 1, sizeof(buf), f) > 0) {
            if (buf.parent_id == 0)
                return 0;
            bool hash_match = true;
            if ((scmp(buf.type, "dir") == 0) && (buf.parent_id == parent_id)) {
                path_len = slen(path);
                scat(path, "/");
                scat(path, buf.name);
                fprintf(output, "Checking files in %s:\n", path);
                file_counter = check_integrity(
                        path, parent_id, data, f, input, buf, output);
                fprintf(output, "\n");
                *(path + path_len) = '\0';
                for (int i = 0; i < file_counter; i++)
                    fread(&buf, 1, sizeof(buf), f);
            } else if (
                    (scmp(buf.type, "file") == 0)
                    && (buf.parent_id == parent_id)) {
                path_len = slen(path);
                scat(path, "/");
                scat(path, buf.name);
                input = fopen(path, "rb");

                if (input == NULL) {
                    fprintf(output, "%s - DELETED\n", buf.name);
                    file_counter++;
                    *(path + path_len) = '\0';
                } else {
                    get_hash(input, &record);

                    *(path + path_len) = '\0';

                    for (int i = 0; i < 16; i++) {
                        if (buf.hash[i] != record.hash[i]) {
                            hash_match = false;
                            break;
                        }
                    }
                    if (hash_match == true) {
                        fprintf(output, "%s - OK\n", buf.name);
                        file_counter++;
                    } else {
                        fprintf(output, "%s - CHANGED\n", buf.name);
                        file_counter++;
                    }
                }
            } else
                return file_counter;
        }
    } else
        return -7;
    //БД пустая
    return file_counter;
}

int main(int argc, char* argv[])
{
    DIR* dir;
    FILE* f;
    FILE* input;
    FILE* output;
    Record buf;

    output = fopen("report.txt", "w");

    FILE* data;
    if ((data = fopen("database.bin", "r+b")) == NULL) {
        data = fopen("database.bin", "w+b");
        rewind(data);
    }
    char* database = NULL;
    bool is_nested = false;
    unsigned int id = 1;
    if ((argc < 5) && (argc > 6)) {
        printf("Usage:\n");
        printf("./integrctrl -s -f database <path to file or directory>\n");
        printf("./integrctrl -s -r -f database <path to file or "
               "directory>\n");
        printf("./integrctrl -c -f database <path to file or directory>\n");
        return -1;
    }
    /*printf("%s\n %s\n %s\n %s\n %s\n",
           argv[0],
           argv[1],
           argv[2],
           argv[3],
           argv[4]);*/

    if (((scmp(argv[1], "-s") != 0) && (scmp(argv[1], "-c") != 0))) {
        printf("Expected -s or -c as first argument\n");
        return -2;
    }
    if ((scmp(argv[2], "-f") != 0) && (scmp(argv[2], "-r") != 0)) {
        printf("Expected -f or -r as second argument\n");
        return -3;
    }
    if ((scmp(argv[2], "-r") == 0) && (scmp(argv[3], "-f") != 0)) {
        printf("Expected -f as third argument\n");
        return -4;
    }

    if ((scmp(argv[2], "-r") == 0) && (scmp(argv[3], "-f") == 0)) {
        is_nested = true;
        save_info(argv[5], is_nested, &id, 0, data, dir);
        return 0;
    }

    if ((scmp(argv[1], "-s") == 0) && (scmp(argv[2], "-f") == 0)
        && (scmp(argv[3], "database") == 0)) {
        save_info(argv[4], database, &id, 0, data, dir);
    }
    if ((scmp(argv[1], "-c") == 0) && (scmp(argv[2], "-f") == 0)
        && (scmp(argv[3], "database") == 0)) {
        check_integrity(argv[4], 0, data, f, input, buf, output);
        return 0;
    }
    fclose(input);
    fclose(output);
    closedir(dir);
    fclose(f);
    fclose(data);
    return 0;
}
