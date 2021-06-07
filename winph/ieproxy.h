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

#ifndef IEPROXY_H
#define IEPROXY_H

#include <windows.h>
#include "winphdef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

BOOL SetIEProxy(UINT uFlag, wpString szProxyUrl, 
                wpString szScriptUrl, wpString szByPass);

BOOL QueryIEProxy(UINT *uFlag, wpString szProxyUrl,
                wpString szScriptUrl, wpString szByPass);

#ifdef __cplusplus
}
#endif /* __cpusplus */

#endif /* IEPROXY_H */