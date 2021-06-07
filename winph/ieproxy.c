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

#include "ieproxy.h"
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

BOOL SetIEProxy(UINT uFlag, wpString szProxyUrl,
                wpString szScriptUrl, wpString szByPass)
{
    INTERNET_PER_CONN_OPTION_LIST coList;
    BOOL bReturn;
    DWORD dwBufSize = sizeof(coList);

    coList.dwSize = dwBufSize;
    coList.dwOptionCount = 4;
    coList.pszConnection = NULL; /*LAN */
    coList.pOptions = (INTERNET_PER_CONN_OPTION *)malloc(sizeof(INTERNET_PER_CONN_OPTION)*4);
    if(coList.pOptions == NULL)
    {
        return FALSE;
    }
    coList.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    coList.pOptions[0].Value.dwValue = uFlag | PROXY_TYPE_DIRECT;

    coList.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    coList.pOptions[1].Value.pszValue = szProxyUrl;

    coList.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    coList.pOptions[2].Value.pszValue = szByPass;

    coList.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
    coList.pOptions[3].Value.pszValue = szScriptUrl;

    bReturn = InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION,
        &coList, dwBufSize);
    free(coList.pOptions);
    if(bReturn == ERROR_SUCCESS) 
    {
        InternetSetOption(NULL, INTERNET_OPTION_PROXY_SETTINGS_CHANGED, NULL, NULL);
    }
    return bReturn;
}

BOOL QueryIEProxy(UINT* uFlag, wpString szProxyUrl, 
                  wpString szScriptUrl, wpString szByPass)
{
    INTERNET_PER_CONN_OPTION_LIST coList;
    BOOL bReturn;
    DWORD dwBufSize = sizeof(coList);

    coList.dwSize = dwBufSize;
    coList.pszConnection = NULL;
    coList.dwOptionCount = 4;
    coList.pOptions = (INTERNET_PER_CONN_OPTION *)malloc(sizeof(INTERNET_PER_CONN_OPTION)*4);
    if(coList.pOptions == NULL)
    {
        return FALSE;
    }
    coList.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    
    coList.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;

    coList.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;

    coList.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;

    bReturn = InternetQueryOption(NULL,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &coList, &dwBufSize);

    StrCpy(szProxyUrl, coList.pOptions[1].Value.pszValue);
    StrCpy(szScriptUrl, coList.pOptions[3].Value.pszValue);
    StrCpy(szByPass, coList.pOptions[2].Value.pszValue);
    *uFlag = coList.pOptions[0].Value.dwValue;

    free(coList.pOptions);

    return bReturn;
}
