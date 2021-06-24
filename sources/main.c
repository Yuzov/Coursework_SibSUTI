#include "functions.h"

int save_info(
        char* path,
        int* is_nested,
        unsigned int* id,
        unsigned int parent_id,
        FILE* data,
        FILE* input,
        DIR* dir)
{
    Record record;
    bool direct = true;
    char* ptr[PTR_SIZE];
    int nesting_cnt;
    int path_len;

    if (check_dir(path, input, &record, &direct) != 0) {
        printf("Enter path to directory\n");
        return -7;
    }
    nesting_cnt = stok(path, '/', ptr);
    scopy(ptr[nesting_cnt - 1] + 1, record.name);
    suntok(path, '/', ptr, nesting_cnt);

    if (*is_nested < 3) {
        //Проверка, что данная директория уже добавлена
        *is_nested = *is_nested + 1;
        fseek(data, 0, SEEK_END);
        unsigned int pos = ftell(data);
        if (pos > 0) {
            FILE* f;
            f = fopen("database.bin", "r+b");
            if (f == NULL) {
                printf("Can not open database.bin\n");
                return -8;
            }
            while (!feof(f)) {
                Record buf;
                if (fread(&buf, 1, sizeof(buf), f) > 0) {
                    *id = *id + 1;
                    if ((scmp(buf.name, record.name) == 0)
                        && (buf.parent_id == record.parent_id)
                        && (scmp(buf.type, record.type) == 0)) {
                        printf("This directory is already recorded\n");
                        printf("Do you want to overwrite? Enter <y> or <n>\n");
                        char answer;
                        scanf("%c", &answer);
                        if (answer == 'n')
                            return 14;
                        else if (answer == 'y') {
                            fseek(data, (*id - 2) * sizeof(buf), SEEK_SET);
                            scopy("\0", buf.name);
                            fwrite(&buf, 1, sizeof(buf), data);
                            save_info(
                                    path,
                                    is_nested,
                                    id,
                                    parent_id,
                                    data,
                                    input,
                                    dir);
                            return 0;
                        } else {
                            printf("Wrong answer\n");
                            return -15;
                        }
                    }
                }
            }
            fclose(f);
        }
    }
    //Запись данной директории
    record.id = *id;
    record.parent_id = parent_id;
    fwrite(&record, 1, sizeof(record), data);

    if (direct == true) {
        dir = opendir(path);
        if (dir == NULL) {
            printf("Can not open path %s\n", path);
            return -9;
        }
        struct dirent* entity;
        entity = readdir(dir);

        while (entity != NULL) {
            if ((entity->d_type == DT_DIR) && (scmp(entity->d_name, ".") != 0)
                && (scmp(entity->d_name, "..") != 0) && (*is_nested > 1)) {
                concat_name(&path_len, path, entity->d_name);
                *id = *id + 1;
                save_info(path, is_nested, id, parent_id + 1, data, input, dir);
                *(path + path_len) = '\0';
            }
            if ((entity->d_type == DT_DIR) && (*is_nested == 1)
                && (scmp(entity->d_name, ".") != 0)
                && (scmp(entity->d_name, "..") != 0)) {
                scopy(entity->d_name, record.name);
                scopy("dir", record.type);
                fwrite(&record, 1, sizeof(record), data);
            }
            if (entity->d_type == DT_REG) {
                record.id++;
                record.parent_id = parent_id + 1;
                scopy(entity->d_name, record.name);
                scopy("file", record.type);
                concat_name(&path_len, path, record.name);
                input = fopen(path, "rb");
                if (input != NULL) {
                    get_hash(input, &record);

                    *(path + path_len) = '\0';

                    fwrite(&record, 1, sizeof(record), data);

                    // printf("%hhd %s\n", entity->d_type, entity->d_name);
                    *id = *id + 1;
                } else {
                    printf("Can not open %s\n", path);
                    return -10;
                }
            }
            entity = readdir(dir);
        }
    }
    fclose(input);
    closedir(dir);
    return 0;
}
int check_integrity(
        char* path,
        unsigned int parent_id,
        FILE* data,
        FILE* f,
        FILE* input,
        Record buf,
        FILE* output,
        DIR* dir)
{
    int path_len;
    Record record;
    record.id = 1;
    record.parent_id = parent_id;
    int file_counter = 0;

    if (*(path + slen(path) - 1) == '/')
        *(path + slen(path) - 1) = '\0';

    dir = opendir(path);
    char* ptr[PTR_SIZE];
    int nesting_cnt = stok(path, '/', ptr);
    scopy((ptr[nesting_cnt - 1] + 1), record.name);
    suntok(path, '/', ptr, nesting_cnt);

    if (dir == NULL) {
        fprintf(output,
                "directory %s - was deleted at path: %s\n",
                record.name,
                path);
        closedir(dir);
        return 0; // Был удален
    } else {
        fprintf(output,
                "directory %s - exists at path: %s\n",
                record.name,
                path);
    }
    closedir(dir);
    scopy("dir", record.type);
    if ((data = fopen("database.bin", "rb")) == NULL) {
        printf("Can not open database.bin\n");
        return -11; //БД отсутствует
    }
    fseek(data, 0, SEEK_END);
    unsigned int pos = ftell(data);
    if (pos > 0) {
        fseek(data, 0, SEEK_SET);
        f = fopen("database.bin", "r+b");
        if (f == NULL) {
            printf("Can not open database.bin\n");
            return -12;
        }
        while (!feof(f)) {
            if (fread(&buf, 1, sizeof(buf), f) > 0) {
                if ((scmp(buf.name, record.name) == 0)
                    && (buf.parent_id == record.parent_id)) {
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
                concat_name(&path_len, path, buf.name);
                // fprintf(output, "Checking files in %s:\n", path);
                file_counter = check_integrity(
                        path, parent_id, data, f, input, buf, output, dir);
                // fprintf(output, "\n");
                *(path + path_len) = '\0';
                for (int i = 0; i < file_counter; i++)
                    fread(&buf, 1, sizeof(buf), f);
            } else if (
                    (scmp(buf.type, "file") == 0)
                    && (buf.parent_id == parent_id)) {
                concat_name(&path_len, path, buf.name);
                input = fopen(path, "rb");

                if (input == NULL) {
                    fprintf(output,
                            "file %s - DELETED at path: %s\n",
                            buf.name,
                            path);
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
                        fprintf(output,
                                "file %s - OK at path: %s\n",
                                buf.name,
                                path);
                        file_counter++;
                    } else {
                        fprintf(output,
                                "file %s - CHANGED at path: %s\n",
                                buf.name,
                                path);
                        file_counter++;
                    }
                }
            } else
                return file_counter;
        }
    } else {
        printf("Database is empty\n");
        return -13;
    }
    if (parent_id == 1) {
        fclose(input);
        closedir(dir);
        fclose(f);
    }
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
    if (output == NULL) {
        printf("Can not open report.txt\n");
        return -14;
    }

    FILE* data;
    if ((data = fopen("database.bin", "r+b")) == NULL) {
        data = fopen("database.bin", "w+b");
        if (data == NULL) {
            printf("Can not open database.bin\n");
            return -15;
        }
        rewind(data);
    }
    int is_nested = 0;
    unsigned int id = 1;
    if ((argc < 5) || (argc > 6)) {
        printf("Usage:\n");
        printf("./integrctrl -s -f database <path to file or directory>\n");
        printf("./integrctrl -s -r -f database <path to file or "
               "directory>\n");
        printf("./integrctrl -c -f database <path to file or directory>\n");
        return -1;
    }
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

    if ((scmp(argv[3], "database") != 0) && (scmp(argv[4], "database") != 0)) {
        printf("Expected database after keys\n");
    }

    if ((scmp(argv[1], "-s") == 0) && (scmp(argv[2], "-r") == 0)
        && (scmp(argv[3], "-f") == 0) && (scmp(argv[4], "database") == 0)) {
        is_nested = 2;
        if (save_info(argv[5], &is_nested, &id, 0, data, input, dir) == 0) {
            printf("Saved in database\n");
            fclose(data);
            return 0;
        } else
            return -5;
    }

    if ((scmp(argv[1], "-s") == 0) && (scmp(argv[2], "-f") == 0)
        && (scmp(argv[3], "database") == 0)) {
        if (save_info(argv[4], &is_nested, &id, 0, data, input, dir) == 0) {
            printf("Saved in database\n");
            fclose(data);
            return 0;
        } else
            return -6;
    }
    if ((scmp(argv[1], "-c") == 0) && (scmp(argv[2], "-f") == 0)
        && (scmp(argv[3], "database") == 0)) {
        check_integrity(argv[4], 0, data, f, input, buf, output, dir);
        printf("Result was recorded in report.txt\n");
        fclose(data);
        fclose(output);
        return 0;
    } else {
        printf("Unknown key combination\n");
        printf("Usage:\n");
        printf("./integrctrl -s -f database <path to file or directory>\n");
        printf("./integrctrl -s -r -f database <path to file or "
               "directory>\n");
        printf("./integrctrl -c -f database <path to file or directory>\n");
    }
    return 0;
}
