#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char * cstrptr;
#define _4MB_ (4*1024*1024)
#define _8MB_ (2*_4MB_)


#define PREFIX	("<item id=\"")
#define PREFIXLEN	(10)

int process(cstrptr, cstrptr, cstrptr);

int main(int argc, char **argv) {
	int i;
	cstrptr buf1, buf2;
	if (argc < 2) {
		printf("Usage: resmgr <filename> ...\n");
		return 0;
	}
	buf1 = (char*)malloc(_8MB_);
	if (buf1 == NULL) {
		printf("Cannot allocate memory!\n");
		return 1;
	}
	buf2 = buf1 + _4MB_;
	for (i = 1; i < argc; ++i) {
		process(argv[i], buf1, buf2);
	}
	free(buf1);
}

int process(cstrptr fn, cstrptr bufi, cstrptr bufo) {
	static int called = 0;
	FILE *fin, *fout, *fh;
	int finSize, foutSize;
	cstrptr pi, po, ph;
	char bufh[512];
	int counter;
	char fno[255] = { 0 };
	fin = fout = NULL;
	fin = fopen(fn, "rb");
	if (fin == NULL) {
		printf("Error opening %s.\n", fn);
		return 1;
	}
	strcpy(fno, fn);
	strcat(fno, ".rmo");
	fout = fopen(fno, "wb");
	if (fout == NULL){
		printf("Cannot write file %s.\n", fno);
		fclose(fin);
		return 1;
	}
	fh = NULL;
	counter = 0;
	if (!called) {
		fh = fopen("header.h", "w");
		if (fh == NULL) {
			printf("cannot write file %s.\n", fh);
			fclose(fin);
			fclose(fout);
			return 1;
		}
		fprintf(fh, "enum RSTR {\n");
	}
	memset(bufi, 0, _4MB_);
	finSize = fread(bufi, sizeof(char), _4MB_, fin);
	fclose(fin);
	pi = bufi;
	po = bufo;
	foutSize = 0;
	while(pi) {
		pi = strstr(pi, PREFIX);
		if (pi != NULL) {
			pi += PREFIXLEN;
			if (fh != NULL) {
				ph = bufh;
				while (*pi != '\"')  {
					*ph++ = *pi++;
				}
				*ph++ = 0;
				if (counter == 0) {
					fprintf(fh, "    %s = 0,\n", bufh);
				}
				else {
					fprintf(fh, "    %s,\n", bufh);
				}
				counter++;
			}
			pi = strstr(pi, "\">");
			pi += 2;
			while (*pi != '<') {
				*po++ = *pi++;
				foutSize++;
			}
			*po++ = 0;
			foutSize++;
		}
	}
	fwrite(bufo, sizeof(char), foutSize, fout);
	if (fh != NULL) {
		fprintf(fh, "    _not_used\n};");
		fclose(fh);
	}
	fclose(fout);
	called = 1;
	return 0;
}

