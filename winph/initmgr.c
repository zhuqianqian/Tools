/*
*  Copyright (C) 2012  Zhu Qianqian (zhuqianqian.299@gmail.com),
*  All Rights Reserved.
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "initmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
  #if (_MSC_VER >= 1400)
    #pragma  warning(disable: 4996)
  #endif
#endif /* _MSC_VER */

#define cpypc(p, q, t, size, exit) \
    if(*(p) > 0x20 && *p < 0x80)\
      *(q)++ = *(p)++;\
    else ++(p);\
    ++(t); \
    if((t) >= (size)) \
    { exit = 1; break; } 

int parse_cfg(const char * stream, long size, cfg_option *list, int count)
{
    unsigned char *p;
    char *q, *w;
    char name[128], fname[128], qname[256];
    int i, j, t, exit, start;
    p = (unsigned char *)stream;
    start = exit = t = 0;
    for(i = 0; i < count && !exit;)
    {
        if(*p == '[')
        {
            ++t; ++p;
            
            q = (char *)name;
            while(*p != ']' && *p != '\n')
            {
                cpypc(p, q, t, size, exit)
            }
            if(exit)
                break;
            *q++ = '\\'; *q = 0; ++p; ++t;
        }
        else if(*p =='\r' || *p == '\n')
        {
            ++p; ++t;
        }
        else if(*p>0x20)
        {
            qname[0] = 0;
            w = (char *)fname;
            while(*p != '=' && *p != '\n')
            {
                cpypc(p, w, t, size, exit)
            }
            if(exit)
                break;
            *w = 0; ++p; ++t;
            strcat(qname, name);
            strcat(qname, fname);
            for(j = start; j < count && !exit; ++j)
            {
                if(strcmp(list[j].name, qname)==0)
                {
                    w = (char*)list[j].rawvalue;
                    while(*p == '\t' || *p == 0x20) {++p; ++t;}
                    list[j].rawvalue = (char *)p;
                    list[j].cb = 0;
                    while(*p != '\r' && *p != '\n')
                    {
                        list[j].cb++;
                        ++p; ++t;
                        if(t >= size)
                        {
                            exit = 1;
                            break;
                        }
                    }
                    if(exit)
                        break;
                    *p = 0; ++p; ++t;
                    ++i;
                    if(start == j) ++start;
                    j = count;
                }
            }
        }
        else
        {
            ++p; ++t;
        }
    }
    return 0;
}

int load_cfg(const char *filename, cfg_option *list, int count, char **buf)
{
    FILE *fp;
    char *content;
    long size;

    fp = fopen(filename, "rb");
    if(fp == NULL)
        return 2;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    content = (char *)malloc(size);
    if(content == NULL)
    {
        fclose(fp);
        return 3;
    }
    fseek(fp, 0, SEEK_SET);
    fread(content, sizeof(char), size, fp);
    fclose(fp);
    parse_cfg((const char*)content, size, list, count);
    *buf = content;
    return 0;
}

int close_cfg(char **buf)
{
    if(buf != NULL)
        free(*buf);
    *buf = NULL;
    return 0;
}

int save_cfg(const char *filename, cfg_option *list, int count)
{
    char current[128], name[128];
    char *w, *p, *q;
    FILE *fp;
    int i, flag;
    fp = fopen(filename, "w");
    if(fp == NULL)
        return 2;
    current[0] = 0;
    for(i = 0; i < count; ++i)
    {
        if(current[0] == 0 || (p = strstr(list[i].name, current)) == NULL)
        {
            w = (char *)current;
            q = (char *)list[i].name;
            while(*q != '\\') {*w++ = *q++;}
            *w = 0;
            q++;
            strcpy(name, q);
            flag = 1;
        }
        else
        {
            flag = 0;
            q = (char *)list[i].name;
            while(*q++ != '\\');
            strcpy(name, q);
        }
        if(flag == 1)
        {
            fprintf(fp, "\n[%s]\n%s = %s\n", current, name, list[i].rawvalue);
        }
        else
        {
            fprintf(fp, "%s = %s\n", name, list[i].rawvalue);
        }
    }
    fclose(fp);
    return 0;
}