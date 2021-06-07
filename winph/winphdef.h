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

#ifndef WINPHDEF_H
#define WINPHDEF_H

#include <stdint.h>
#include <stdlib.h>

#if defined(_MSC_VER)
  #ifdef UNICODE
    #define wpString LPWSTR
    #define StrCpy lstrcpy
    #define StrCmp lstrcmp
    #define StrCat lstrcat
  #else
    #define wpString LPSTR
    #define StrCpy strcpy
    #define StrCmp strcmp
    #define StrCat strcat
  #endif /* UNICODE */
#elif defined(__GNUC__) && (__GNUC__>=2)
  typedef char * wpString;
  #define StrCpy strcpy
  #define StrCmp strcmp
  #define StrCat strcat
#endif /* _MSC_VER */

#endif /* WINPHDEF_H */