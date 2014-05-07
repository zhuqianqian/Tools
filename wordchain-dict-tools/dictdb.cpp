#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

void process(int, char **);


int main(int argc, char **argv) {
    char dbname[128];
    if (argc < 2) {
        printf("dictdb: Generate dictionary database. Type --help for help.\n");
        return 0;
    }
    if (argc < 3) {
        strcpy(dbname, "dict.db");
    }
    else {
        strcpy(dbname, argv[2]);
    }
    process(argc, argv);
}

void handleFile(sqlite3 * db, FILE *in) {
    long size;
    unsigned char *buffer, *w, *p, *end;
    char sql[256];
    char word[64];
    char *send, *s;
    int wlen;
    int rc, count;

    char *error;
    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fseek(in, 0, SEEK_SET);
    buffer = (unsigned char *)malloc(size);
    if (buffer != NULL) {
        fread(buffer, 1, size, in);
    }
    end = buffer + size;
    p = buffer;
    strcpy(sql, "INSERT INTO English (word) VALUES ( \"");
    send = &sql[strlen(sql)];
    count = 0;
    while (p < end) {
        while (*p < 0x20) p++;
        if (p > end) break;
        wlen = 0;
        do {
            if (*p == 0x0d && *(p + 1) == 0x0a)
            {
                p++; p++;
                while (*p>0x20) p++;
                if (*p == 0x0d && *(p+1) == 0x0a
                    &&*(p+2) == 0x0d && *(p+3) == 0x0a) {
                    p += 4;
                }
                break;
            }
            else {
                word[wlen++] = *p;
            }
            p++;
        } while (1);
        word[wlen] = 0;
        if (wlen <32 && wlen > 1) {
            strcpy(send, word);
            strcat(send, "\");");
            rc = sqlite3_exec(db, sql, NULL, 0, &error);
            if (rc != SQLITE_OK) {
                printf("SQL Error:%s.\n", error);
                sqlite3_free(error);
            }
            else {
                count++;
                if (count % 1000 == 0) {
                    //printf("New added words: %d.\n", count);
                }
            }
        }
        
    }
    printf("New added words: %d.\n", count);
    free(buffer);
}

void handleFile2(sqlite3 * db, FILE *in) {
    long size;
    unsigned char *buffer, *w, *p, *end;
    char sql[256];
    char word[64];
    char *send, *s;
    int wlen;
    int rc, count;

    char *error;
    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fseek(in, 0, SEEK_SET);
    buffer = (unsigned char *)malloc(size);
    if (buffer != NULL) {
        fread(buffer, 1, size, in);
    }
    end = buffer + size;
    p = buffer;
    strcpy(sql, "INSERT INTO English (word) VALUES ( \"");
    send = &sql[strlen(sql)];
    count = 0;
    while (p < end) {
        while (*p < 0x20) p++;
        if (p > end) break;
        wlen = 0;
        do {
            if(*p>='a' && *p <= 'z'){
                word[wlen++] = *p;
                p++;
            }
            else {
                while (*p != 0x0a) p++;
            }
        } while (*p != 0x0a);
        word[wlen] = 0;
        if (wlen <32 && wlen > 1) {
            strcpy(send, word);
            strcat(send, "\");");
            rc = sqlite3_exec(db, sql, NULL, 0, &error);
            if (rc != SQLITE_OK) {
                printf("SQL Error:%s.\n", error);
                sqlite3_free(error);
            }
            count++;
            if ((count % 1000) == 0) {
                printf("Processed words: %d\n", count);
            }
        }

    }
    free(buffer);
}


void process(int argc, char **argv) {
    FILE * fin;
    sqlite3 * db;
    int i, rc;
    char sqlstmt[1024];
    char *error;
    rc = sqlite3_open("dict.db", &db);
    if (rc) {
        printf("Cannot open database dict.db.\n");
        sqlite3_close(db);
        return;
    }
    strcpy(sqlstmt, "CREATE TABLE English (word VARCHAR(24) PRIMARY KEY);");
    rc = sqlite3_exec(db, sqlstmt, NULL, 0, &error);
    if (rc != SQLITE_OK) {
        printf("SQL error:%s.\n", error);
        sqlite3_free(error);
    }
    for (i = 1; i < argc; ++i) {
        fin = fopen(argv[i], "rb");
        if (fin == NULL) {
            printf("Cannot open file %s.\n", argv[i]);
        }
        else {
            printf("Process file %s...\n", argv[i]);
            handleFile(db, fin);
            fclose(fin);
        }
    }
    sqlite3_close(db);
}