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

#ifndef INITMGR_H
#define INITMGR_H

#include <stdint.h>

typedef struct _cfg_option
{
    char * name;
    char * rawvalue;
    size_t cb;
} cfg_option;

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */

int load_cfg(const char *filename, cfg_option *list, int count, char **buf);

int close_cfg(char **buf);

int save_cfg(const char *filename, cfg_option *list, int count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INITMGR_H */
