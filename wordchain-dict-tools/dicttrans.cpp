#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3.h"

/* This is used to parse the word definition from dict.cn */

#define _128K_  (128*1024)

#define UNFIND  ("section unfind")
#define DEFINITION ("section def")
#define OPEN_LI  ("<li>")
#define CLOSE_LI ("</li>")
#define OPEN_SPAN ("<span>")
#define CLOSE_SPAN ("</span>")
#define OPEN_STRONG ("<strong>")
#define CLOSE_STRONG ("</strong>")

#define OPEN_LI_LEN (4)
#define OPEN_SPAN_LEN (6)
#define OPEN_STRONG_LEN (8)

char forbidden[][8] = {"aux", "con", "prn", "nul"};

char *buffer;

int get_trans(char * word, char *exp) {
	int size;
	char filename[128];
	char *s, *p, *q, *w, *opening, *closing, *x;
	filename[0] = *word;
	filename[1] = '\\';
	filename[2] = 0;
	strcat(filename, word);
	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		return 1;
	}
	if (strcmp(word, "aux") == 0 || strcmp(word, "con") == 0 || strcmp(word, "prn") == 0 || strcmp(word, "nul") == 0) {
		return 1;
	}
	size = fread(buffer, 1, _128K_, file);
	fclose(file);
	buffer[size - 1] = 0;
	s = strstr(buffer, UNFIND);
	if (s != NULL) {
		return 1;
	}
	s = strstr(buffer, DEFINITION);
	if (s != NULL) {
		s = strstr(s, "layout basic clearfix");
		if (s == NULL) {
			return 1;
		}
		p = strstr(s, "</ul>");
		*p = 0;
		x = (char *)exp;
		while (1) {
			q = strstr(s, OPEN_LI);
			if (q == NULL) break;
			w = strstr(q, CLOSE_LI);
			opening = strstr(q, OPEN_SPAN);
			closing = strstr(opening, CLOSE_SPAN);
			for (opening += OPEN_SPAN_LEN; opening < closing; opening++) {
				*x = *opening;
				x++;
			}
			*x++ = 0x20;
			opening = strstr(opening, OPEN_STRONG);
			closing = strstr(opening, CLOSE_STRONG);
			for (opening += OPEN_STRONG_LEN; opening < closing; opening++) {
				*x = *opening;
				x++;
			}
			*x++ = '\n';
			s = w;
		}
		x--;
		*x = 0;
		return 0;
	}
	return 1;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Specify an initial letter\n");
		return 0;
	}
	FILE * log = fopen("dict.log", "a");
	sqlite3 *dbr, *dbw;
	char * error;
	int i, j;
	sqlite3_stmt *res;
	char word[64];
	char meaning[512];
	const char *tail;
	sqlite3_open("dict.db", &dbr);
	sqlite3_open("dicttrans.db", &dbw);
	char sqlquery[256] = { 0 };
	char sqlinsert[1024] = { 0 };
	strcpy(sqlquery, "SELECT WORD FROM English WHERE WORD LIKE \"");
	strcat(sqlquery, argv[1]);
	strcat(sqlquery, "%\"");
	sqlite3_prepare_v2(dbr, sqlquery, 1000, &res, &tail);
	buffer = (char *)malloc(_128K_);
	i = j = 0;
	while (sqlite3_step(res) == SQLITE_ROW) {
		word[0] = meaning[0] = 0;
		i++;
		strcpy(word, (const char *)sqlite3_column_text(res, 0));
		if (0 == get_trans(word, meaning)) {
			sqlinsert[0] = 0;
			strcpy(sqlinsert, "INSERT INTO English (word, def) VALUES (\"");
			strcat(sqlinsert, word);
			strcat(sqlinsert, "\", \"");
			strcat(sqlinsert, meaning);
			strcat(sqlinsert, "\")");
			sqlite3_exec(dbw, sqlinsert, NULL, NULL, &error);
		}
		else {
			j++;
			fprintf(log, "%s\n", word);
		}
		printf("Total processed: %d, total not found: %d.\r", i, j);
	}
	printf("\n");
	sqlite3_free(error);
	free(buffer);
	sqlite3_close(dbr);
	sqlite3_close(dbw);
	fclose(log);
	return 0;
}

