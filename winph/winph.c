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
#include "ieproxy.h"
#include "conn.h"
#include "resource.h"
#include <stdlib.h>
#include <wininet.h>
#include <commctrl.h>
#include <shellapi.h>
#include <process.h>
#include <stdio.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")

#if defined(_MSC_VER)
  #if (_MSC_VER >= 1400)
    #pragma  warning(disable: 4996)
  #endif
#endif /* _MSC_VER */

#define IDC_LV    (299)
#define WINPH_TNID    (299)
#define MENUACSBASEID (40299)
#define MENUPSBASEID  (41299)
#define WM_TASKBAR (WM_USER+1)
#define WM_CREATETHREADS (WM_USER+2)
#define WM_UPDATERT (WM_USER+3)


/* Global variables or constants */
HINSTANCE g_hInstance;
HWND g_hDlg;
HWND g_hView;
HICON g_hIcon;
HFONT g_hFontTitle;
HFONT g_hFontSmall;
UINT  g_rcDlgXY[2];
wpString szClass = TEXT("WinphHWND");
TCHAR szTitle[128];
TCHAR szNA[8];
TCHAR szString[3][64];
NOTIFYICONDATA nid;
UINT uPostMessage;
char filename[260];

typedef struct _thread_data
{
    int count;
    HANDLE * hThreads;
    HANDLE * hEvents;
    int exit;
} ThreadData;

ThreadData td;

char *cfg_buf;
cfg_option options[] = 
{
    {"Window\\Size", "38666854", 8},
    {"Window\\Pos", "8388736", 7},
    {"Window\\Mode", "1", 1},
    {"Setting\\Language", "EN", 2},
    {"Setting\\Thread", "5", 1},
    {"Proxy\\AutoScript", "", 0},
    {"Proxy\\ProxyServer", "", 0},
    {"Proxy\\Bypass", "", 0},
    {"Proxy\\Exlusive", "1", 1},
    {"Proxy\\RefreshFrequency", "1", 1}
};

wpString szUrls[] = {
    TEXT("http://code.google.com/p/windows-system-proxy-helper/w/list"),
    TEXT("http://code.google.com/p/windows-system-proxy-helper/issues/list")
};

typedef struct _app_option
{
    USHORT xWidth, yHeight;
    USHORT xWin, yWin;
    int nCmdShow;
    int nExlusive;
    wpString szScript;
    wpString szProxy;
    
    wpString szBypass[256];

    int nUseScript;
    int nUseProxy;
    int nAutoStart;
    int nRefresh;
    int nThread;
} APPOPTION;

typedef struct _app_data
{
    char *buffer;
    char *nextalloc;
    size_t size;
    size_t used;

    wpString * scripts;
    wpString * servers;
    char *scriptflag;
    char *serverflag;
    int scriptind, serverind;
    int scriptuse, serveruse;
} APPDATA;

APPOPTION app;
APPDATA ad;

#define add_one_script(s, p, w) {\
    p = (wpString)ad.nextalloc; \
    ad.scripts[ad.scriptind++] = (wpString)p;\
    w = (wpString)s; \
    while(*p++ = *w++) ad.used += sizeof(TCHAR);\
    ad.nextalloc = (char*)p;}

#define add_one_server(s, p, w) {\
    p = (wpString)ad.nextalloc; \
    ad.servers[ad.serverind++] = (wpString)p;\
    w = (wpString)s; \
    while(*p++ = *w++) ad.used += sizeof(TCHAR);\
    ad.nextalloc = (char*)p;}


BOOL InitWindow(HINSTANCE hInstance, wpString szTitle);
LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);
int HandleInit();
int SaveInit();

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    
    WSADATA wd;
    HWND hwnd;
    MSG msg;
    LoadString(hInstance, IDS_TITLE, szTitle, 128);
    LoadString(hInstance, IDS_DIRECT, szString[0], 128);
    LoadString(hInstance, IDS_AUTOSCRIPT, szString[1], 128);
    LoadString(hInstance, IDS_PROXY, szString[2], 128);
    LoadString(hInstance, IDS_VIEW_NA, szNA, 8);
    hwnd = FindWindow(szClass, szTitle);
    if(hwnd != NULL)
    {
        ShowWindow(hwnd, SW_RESTORE);
        SetForegroundWindow(hwnd);
        return 0;
    }
    WSAStartup(0x202, &wd);
    HandleInit();
    if(strcmp(lpCmdLine, "--auto")==0)
    {
        app.nCmdShow = SW_HIDE;
    }
    g_hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
    g_hInstance = hInstance;

    InitWindow(hInstance, (wpString)szTitle);

    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(!IsDialogMessage(g_hDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    SaveInit();
    free(ad.buffer);
    WSACleanup();
    return msg.wParam;
}
int HandleInit()
{
    char *w;
    TCHAR *p;
    UINT uInt;
    cfg_buf = NULL;
    uInt = GetModuleFileNameA(NULL, filename, 260);
    w = filename + uInt - 3;
    *w = 'i';
    *(w+1) = 'n';
    *(w+2) = 'i';
    load_cfg(filename, options, 10, &cfg_buf);
    uInt = (UINT)atoi(options[0].rawvalue);
    app.xWidth = HIWORD(uInt);
    app.yHeight = LOWORD(uInt);
    uInt = (UINT)atoi(options[1].rawvalue);
    app.xWin = HIWORD(uInt);
    app.yWin = LOWORD(uInt);
    app.nCmdShow = atoi(options[2].rawvalue);
    app.nThread = atoi(options[4].rawvalue);
    app.nExlusive = atoi(options[8].rawvalue);
    app.nRefresh = atoi(options[9].rawvalue);
    
    ad.size = 2048*sizeof(void *);
    ad.buffer = (char*)malloc(ad.size); /* 8K or 16K */
    ad.scripts = (wpString *)(ad.buffer);
    ad.servers = (wpString *)(ad.buffer+sizeof(void*)*64);
    ad.scriptflag = (char*)(ad.servers+64);
    ad.serverflag = ad.scriptflag + 64;
    ad.scriptind = 0;
    ad.serverind = 0;
    ad.nextalloc = ad.buffer+2048;

    if(options[5].cb > 0)
    {
        w = options[5].rawvalue;
        p = (wpString)ad.nextalloc;
        while(*w)
        {
            if(*w == '|')
            {
                *p = 0;
                ad.scripts[ad.scriptind++] = (wpString)ad.nextalloc;
                ad.nextalloc = (char *)(p+1);
                ++p; ++w;
            }
            else *p++ = *w++;
        }
        *p = 0;
        ad.scripts[ad.scriptind++] = (wpString)ad.nextalloc;
        ad.nextalloc = (char *)(p+1);
    }
    if(options[6].cb>0)
    {
        w = options[6].rawvalue;
        p = (wpString)ad.nextalloc;
        while(*w)
        {
            if(*w == '|')
            {
                *p = 0;
                ad.servers[ad.serverind++] = (wpString)ad.nextalloc;
                ad.nextalloc = (char *)(p+1);
                ++p; ++w;
            }
            else *p++ = *w++;
        }
        *p = 0;
        ad.servers[ad.serverind++] = (wpString)ad.nextalloc;
        ad.nextalloc = (char *)(p+1);        
    }
    if(cfg_buf != NULL)
        close_cfg(&cfg_buf);
    return 0;
}

int SaveInit()
{
    char *p;
    wpString w;
    int cb, i;
    UINT v;
    cfg_buf = (char*)malloc(ad.size);
    p = cfg_buf;
    v = MAKELPARAM(app.yHeight, app.xWidth);
    cb = sprintf(p, "%d", v);
    options[0].rawvalue = p;
    options[0].cb = cb;
    p = p+cb+1;
    v = MAKELPARAM(app.yWin, app.xWin);
    cb = sprintf(p, "%d", v);
    options[1].rawvalue = p;
    options[1].cb  = cb;
    p = p + cb+ 1;
    cb = sprintf(p, "%d", SW_SHOWNORMAL);
    options[2].rawvalue = p;
    options[2].cb = cb;
    p = p +cb + 1;
    strcpy(p, "EN");
    options[3].rawvalue = p;
    options[3].cb = 2;
    p = p + 3;
    cb = sprintf(p, "%d", app.nThread);
    options[4].rawvalue = p;
    options[4].cb =  cb;
    p = p + cb +1;
    options[5].rawvalue = p;
    for(cb = i = 0; i < ad.scriptind; ++i)
    {
        w = ad.scripts[i];
        while(*w) { *p++ = *w++; cb++;}
        *p = '|'; cb++; p++;
    }
    *(p-1) = 0; cb -= 1;
    options[5].cb = cb;
    options[6].rawvalue = p;
    for(cb = i = 0; i < ad.serverind; ++i)
    {
        w = ad.servers[i];
        while(*w) {*p++ = *w++; cb++;}
        *p = '|'; cb++; p++;
    }
    *(p-1) = 0; cb -= 1;
    options[6].cb = cb;
    options[7].cb = 0;
    options[7].rawvalue = p-1;
    cb = sprintf(p, "%d", app.nExlusive);
    options[8].rawvalue = p;
    options[8].cb = cb;
    p = p + cb + 1;
    cb = sprintf(p, "%d", app.nRefresh);
    options[9].rawvalue = p;
    options[9].cb = cb;
    save_cfg(filename, options, 10);
    free(cfg_buf);
    return 0;
}

BOOL QueryAutoStartup()
{
    DWORD dw;
    HKEY hKey;

    RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"),
        0, 0, 0, KEY_READ, 0, &hKey, &dw);
    dw = 256;
    if(ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("winph"), NULL, NULL, NULL, NULL))
        return TRUE;
    return FALSE;
}

BOOL SetAutoStartup()
{
    DWORD dw;
    HKEY hKey;
    TCHAR szInfo[256] = {0};

    GetModuleFileName(NULL, szInfo, 256);
    StrCat(szInfo, TEXT(" --auto"));
    RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"),
        0, 0, 0, KEY_WRITE, 0, &hKey, &dw);
    if(app.nAutoStart == 1)
    {
        RegSetValueEx(hKey, TEXT("winph"), 0, REG_SZ,
            (LPBYTE)szInfo, lstrlen(szInfo)*sizeof(TCHAR));
    }
    else
        RegDeleteValue(hKey, TEXT("winph"));
    RegCloseKey(hKey);
    return TRUE;
}

BOOL InitWindow(HINSTANCE hInstance, wpString szTitle)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = g_hIcon;
    wcex.hIconSm        = g_hIcon;
    wcex.hCursor        = LoadCursor(hInstance, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
    wcex.lpszClassName    = szClass;
    wcex.lpszMenuName    = NULL;

    if(!RegisterClassEx(&wcex))
        return FALSE;

    hwnd = CreateWindowEx(WS_EX_WINDOWEDGE, szClass, szTitle, 
                          WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_SIZEBOX,
                          app.xWin, app.yWin, app.xWidth, app.yHeight,
                          NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, app.nCmdShow);
    return TRUE;
}

BOOL WINAPI CreateView(HWND hDlg)
{
    LVCOLUMN lvcol;
    LVITEM lvi;
    int i;
    RECT rc1, rc2;
    wpString p, w;
    TCHAR szTexts[3][32];
    int cxArray[3] = {220, 100, 140};
    LoadString(g_hInstance, IDS_VIEW_H1, szTexts[0], 32);
    LoadString(g_hInstance, IDS_VIEW_H2, szTexts[1], 32);
    LoadString(g_hInstance, IDS_VIEW_H3, szTexts[2], 32);
    GetWindowRect(hDlg, &rc1);
    GetWindowRect(GetDlgItem(hDlg, IDC_CHECK_PROXY), &rc2);
    g_hView = CreateWindowEx(0, WC_LISTVIEW,TEXT(""), 
        WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | WS_TABSTOP,
        (rc2.left- rc1.left), (rc2.top- rc1.top+70), 460, 150,
        hDlg, (HMENU)IDC_LV, g_hInstance, NULL);
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT;
    SendMessage(g_hView, LVM_SETEXTENDEDLISTVIEWSTYLE, 
        LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    for(i = 0; i < 3;++i)
    {
        lvcol.pszText = szTexts[i];
        lvcol.cx = cxArray[i];
        lvcol.iSubItem = i;
        SendMessage(g_hView, LVM_INSERTCOLUMN, i, (LPARAM)&lvcol);
    }
    LoadString(g_hInstance, IDS_VIEW_UPDATE, szTexts[2], 32);
    lvi.mask = LVIF_TEXT;
    for(i = 0; i < ad.serverind; ++i)
    {
        p = ad.servers[i];
        w = szTexts[0];
        while(*p != ':') *w++ = *p++;
        *w = 0; ++p;
        w = szTexts[1];
        while(*w++ = *p++);
        lvi.iItem = i;
        lvi.pszText = szTexts[0];
        lvi.iSubItem = 0;
        SendMessage(g_hView, LVM_INSERTITEM, 0, (LPARAM)&lvi);
        lvi.pszText = szTexts[1];
        lvi.iSubItem = 1;
        SendMessage(g_hView, LVM_SETITEM, 0, (LPARAM)&lvi);
        lvi.pszText = szTexts[2];
        lvi.iSubItem = 2;
        SendMessage(g_hView, LVM_SETITEM, 0, (LPARAM)&lvi);        
    }    
    return TRUE;
}

BOOL WINAPI InitDlgComponent(HWND hDlg)
{
    TCHAR szScript[256];
    TCHAR szServer[256];

    wpString p, w;
    UINT uFlag;
    int i, the_one;
    HWND hCombo;
    SendMessage(GetDlgItem(hDlg, IDC_STATIC_CS), WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);
    SendMessage(GetDlgItem(hDlg, IDC_STATIC_LIST), WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);
    SendMessage(GetDlgItem(hDlg, IDC_STATIC_APP), WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);
    SendMessage(GetDlgItem(hDlg, IDC_STATIC_HS), WM_SETFONT, (WPARAM)g_hFontTitle, TRUE);
    SendMessage(GetDlgItem(hDlg, IDC_STATIC_ABOUT), WM_SETFONT, (WPARAM)g_hFontSmall, TRUE);
    QueryIEProxy(&uFlag, szServer, szScript, (wpString)app.szBypass);
    app.nAutoStart = QueryAutoStartup();
    if(app.nAutoStart != 0)
    {
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_AUTO), BM_SETCHECK, BST_CHECKED, 0);
    }
    if(szScript[0])
    {
        the_one = -1;
        hCombo = GetDlgItem(hDlg, IDC_COMBO_SCRIPT);
        for(i = 0; i < ad.scriptind; ++i)
        {
            if(StrCmp(szScript, ad.scripts[i])==0)
                the_one = i;
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)ad.scripts[i]);
        }
        if(the_one == -1)
        {
            the_one = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szScript);
            add_one_script(szScript, p, w);
        }
        ad.scriptuse = the_one;
        SendMessage(hCombo, CB_SETCURSEL, (WPARAM)the_one, 0);
        if((uFlag & INTERNET_PER_CONN_AUTOCONFIG_URL)>0)
        {
            SendMessage(GetDlgItem(hDlg, IDC_CHECK_SCRIPT), BM_SETCHECK, BST_CHECKED, 0);
            app.nUseScript = 1;
        }
        else
        {
            app.nUseScript = 0;
            EnableWindow(hCombo, FALSE);
        }
    }
    if(szServer[0])
    {
        the_one = -1;
        hCombo = GetDlgItem(hDlg, IDC_COMBO_PROXY);
        for(i = 0; i < ad.serverind; ++i)
        {
            if(StrCmp(szServer, ad.servers[i])==0)
                the_one = i;
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)ad.servers[i]);
        }
        if(the_one == -1)
        {
            the_one = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szServer);
            add_one_server(szServer, p, w);
        }
        ad.serveruse = the_one;
        SendMessage(hCombo, CB_SETCURSEL, (WPARAM)the_one, 0);        
        if((uFlag & INTERNET_PER_CONN_PROXY_SERVER)>0)
        {
            SendMessage(GetDlgItem(hDlg, IDC_CHECK_PROXY), BM_SETCHECK, BST_CHECKED, 0);
            app.nUseProxy = 1;
        }
        else
        {
            app.nUseProxy = 0;
            EnableWindow(hCombo, FALSE);
        }
    }
    
    if(app.nExlusive == 1)
    {
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_EX), BM_SETCHECK, BST_CHECKED, 0);
        if((uFlag & INTERNET_PER_CONN_PROXY_SERVER)>0)
        {
            SendMessage(GetDlgItem(hDlg, IDC_CHECK_SCRIPT), BM_SETCHECK, BST_UNCHECKED, 0);
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_SCRIPT), FALSE);
            app.nUseScript = 0;
        }
        if((uFlag & INTERNET_PER_CONN_AUTOCONFIG_URL)>0)
        {
            SendMessage(GetDlgItem(hDlg, IDC_CHECK_PROXY), BM_SETCHECK, BST_UNCHECKED, 0);
            EnableWindow(hCombo, FALSE);
            app.nUseProxy = 0;
        }
    }
    wsprintf(nid.szTip, TEXT("%s\r\n%s."), szTitle, szString[0]);
    if(app.nUseProxy)
    {
        wsprintf(nid.szTip, TEXT("%s\r\n%s: %s"),szTitle, szString[2], szServer);
    }
    if(app.nUseScript)
    {
        wsprintf(nid.szTip, TEXT("%s\r\n%s: %s"), szTitle, szString[1], szScript);
    }
    CreateView(hDlg);
    return TRUE;
}

BOOL WINAPI OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    static HWND hCheckProxy = NULL, hCheckScript,
        hComboScript, hComboProxy;
    TCHAR szText[256], szText2[256], szTemp[256];
    UINT uFlag;
    
    if(hCheckProxy==INVALID_HANDLE_VALUE || hCheckProxy == NULL)
    {
        hCheckProxy = GetDlgItem(hDlg, IDC_CHECK_PROXY);
        hCheckScript = GetDlgItem(hDlg, IDC_CHECK_SCRIPT);
        hComboScript = GetDlgItem(hDlg, IDC_COMBO_SCRIPT);
        hComboProxy = GetDlgItem(hDlg, IDC_COMBO_PROXY);
    }
    switch(LOWORD(wParam))
    {
    case IDC_CHECK_EX:
        app.nExlusive = app.nExlusive == 1 ? 0 : 1;
        break;

    case IDC_CHECK_AUTO:
        app.nAutoStart = app.nAutoStart == 1 ? 0 : 1;
        SetAutoStartup();
        break;

    case IDC_CHECK_SCRIPT:
        if(app.nUseScript == 1)
        {
            EnableWindow(hComboScript, FALSE);
            app.nUseScript = 0;
        }
        else {
            EnableWindow(hComboScript, TRUE);
            if(app.nExlusive == 1)
            {
                EnableWindow(hComboProxy, FALSE);
                app.nUseProxy = 0;
                SendMessage(hCheckProxy, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            app.nUseScript = 1;
        }
        GetWindowText(hComboProxy, szText, 256);
        GetWindowText(hComboScript, szText2, 256);
        uFlag = 0;
        if(app.nUseProxy == 0 && app.nUseScript ==0)
            wsprintf(nid.szTip, TEXT("%s\r\n%s."), szTitle, szString[0]);
        else
            wsprintf(nid.szTip, TEXT("%s"), szTitle);
        if(app.nUseProxy)
        {
            uFlag |= INTERNET_PER_CONN_PROXY_SERVER;
            wsprintf(szTemp, TEXT("\r\n%s: %s"), szString[2], szText);
            StrCat(nid.szTip, szTemp);
        }
        if(app.nUseScript )
        {
            uFlag |= INTERNET_PER_CONN_AUTOCONFIG_URL;
            wsprintf(szTemp, TEXT("\r\n%s: %s"), szString[1], szText2);
            StrCat(nid.szTip, szTemp);
        }
        SetIEProxy(uFlag, szText, szText2, (wpString)app.szBypass);
        Shell_NotifyIcon(NIM_MODIFY, &nid);
        break;

    case IDC_CHECK_PROXY:
        if(app.nUseProxy == 1)
        {
            EnableWindow(hComboProxy, FALSE);
            app.nUseProxy = 0;
        }
        else
        {
            EnableWindow(hComboProxy, TRUE);
            if(app.nExlusive == 1)
            {
                EnableWindow(hComboScript, FALSE);
                app.nUseScript = 0;
                SendMessage(hCheckScript, BM_SETCHECK, BST_UNCHECKED, 0);
            }
            app.nUseProxy = 1;
        }
        GetWindowText(hComboProxy, szText, 256);
        GetWindowText(hComboScript, szText2, 256);
        uFlag = 0;
        if(app.nUseProxy == 0 && app.nUseScript ==0)
            wsprintf(nid.szTip, TEXT("%s\r\n%s."), szTitle, szString[0]);
        else
            wsprintf(nid.szTip, TEXT("%s"), szTitle);
        if(app.nUseProxy)
        {
            uFlag |= INTERNET_PER_CONN_PROXY_SERVER;
            wsprintf(szTemp, TEXT("\r\n%s: %s"), szString[2], szText);
            StrCat(nid.szTip, szTemp);
        }
        if(app.nUseScript )
        {
            uFlag |= INTERNET_PER_CONN_AUTOCONFIG_URL;
            wsprintf(szTemp, TEXT("\r\n%s: %s"), szString[1], szText2);
            StrCat(nid.szTip, szTemp);
        }
        SetIEProxy(uFlag, szText, szText2, (wpString)app.szBypass);
        Shell_NotifyIcon(NIM_MODIFY, &nid);
        break;

    case IDC_COMBO_SCRIPT:
    case IDC_COMBO_PROXY:
        if(HIWORD(wParam) == CBN_CLOSEUP)
        {
            ad.serveruse = SendMessage(hComboProxy, CB_GETCURSEL, 0, 0);
            StrCpy(szText, ad.servers[ad.serveruse]);
            ad.scriptuse = SendMessage(hComboScript, CB_GETCURSEL, 0, 0);
            StrCpy(szText2, ad.scripts[ad.scriptuse]);
            uFlag = 0;
            if(app.nUseProxy == 0 && app.nUseScript ==0)
                wsprintf(nid.szTip, TEXT("%s\r\n%s."), szTitle, szString[0]);
            else
                wsprintf(nid.szTip, TEXT("%s"), szTitle);
            if(app.nUseProxy)
            {
                uFlag |= INTERNET_PER_CONN_PROXY_SERVER;
                wsprintf(szTemp, TEXT("\r\n%s: %s"), szString[2], szText);
                StrCat(nid.szTip, szTemp);
            }
            if(app.nUseScript )
            {
                uFlag |= INTERNET_PER_CONN_AUTOCONFIG_URL;
                wsprintf(szTemp, TEXT("\r\n%s: %s"), szString[1], szText2);
                StrCat(nid.szTip, szTemp);
            }
            SetIEProxy(uFlag, szText, szText2, (wpString)app.szBypass);
            Shell_NotifyIcon(NIM_MODIFY, &nid);
        }
        break;
    }
    
    return TRUE;
}

BOOL WINAPI OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch(((LPNMHDR)lParam)->code)
    {
    case NM_CLICK:
    case NM_RETURN:
        if(((LPNMHDR)lParam)->idFrom == IDC_SL_HELP)
            ShellExecute(NULL, TEXT("open"), szUrls[0], NULL, NULL, SW_SHOW);
        else if(((LPNMHDR)lParam)->idFrom == IDC_SL_BUG)
            ShellExecute(NULL, TEXT("open"), szUrls[1], NULL, NULL, SW_SHOW);
        break;
    }
    return TRUE;
}

BOOL WINAPI DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szText[32];
    HBRUSH hBrush;
    switch(msg)
    {
    case WM_INITDIALOG:
        InitDlgComponent(hwnd);
        return TRUE;

    case WM_UPDATERT:
        if(lParam > 0)
        {
            wsprintf(szText, TEXT("%d ms"), lParam);
            ListView_SetItemText(g_hView, wParam, 2, szText);
        }
        else
        {
            ListView_SetItemText(g_hView, wParam, 2, szNA);
        }
        return FALSE;

    case WM_CHAR:
        return 0;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORLISTBOX:
        hBrush = (HBRUSH)GetStockObject(COLOR_WINDOW+1);
        return (INT_PTR)hBrush;

    case WM_COMMAND:
        OnCommand(hwnd, wParam, lParam);
        break;

    case WM_NOTIFY:
        OnNotify(hwnd, wParam, lParam);
        break;
    }
    return FALSE;
}

BOOL AdjustWindow(HWND hwnd)
{
    UINT x, y;
    if(app.xWidth > g_rcDlgXY[0])
        x = (app.xWidth - g_rcDlgXY[0]) >> 1;
    else x = 0;
    y = 10;
    MoveWindow(g_hDlg, x, y, g_rcDlgXY[0], g_rcDlgXY[1], TRUE);
    return TRUE;
}

LRESULT WINAPI OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    g_hFontTitle = CreateFont(17,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                VARIABLE_PITCH,TEXT("Segoe UI"));
    g_hFontSmall = CreateFont(13,0,0,0,FW_THIN,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                VARIABLE_PITCH,TEXT("Arial"));
    InitCommonControls();
    g_hDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PANE), hwnd, (DLGPROC)DlgProc);
    GetWindowRect(g_hDlg, &rect);
    g_rcDlgXY[0] = rect.right - rect.left;
    g_rcDlgXY[1] = rect.bottom - rect.top;
    AdjustWindow(hwnd);
    ShowWindow(g_hDlg, SW_SHOWNORMAL);
    nid.cbSize = sizeof(nid);
    nid.uID = WINPH_TNID;
    nid.hIcon = g_hIcon;
    nid.uCallbackMessage = WM_TASKBAR;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.hWnd = hwnd;
    Shell_NotifyIcon(NIM_ADD, &nid);
    uPostMessage = RegisterWindowMessage(TEXT("TaskbarCreated"));
    return 0;
}

BOOL OnCreateMenu(HWND hwnd, WPARAM wParam)
{
    HMENU hMenu, hScriptMenu, hProxyMenu, hSettingMenu;
    HMENU g_hMenu;
    UINT uCheck;
    POINT pt;
    DWORD dw;
    int i;    
    g_hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU));
    hMenu = GetSubMenu(g_hMenu, 0);
    hScriptMenu = GetSubMenu(hMenu, 1);
    hProxyMenu = GetSubMenu(hMenu, 2);
    hSettingMenu = GetSubMenu(hMenu, 4);
    if(app.nUseProxy == 0 && app.nUseScript == 0)
    { 
        CheckMenuItem(hMenu, ID_DIRECT_CONNECT, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        if(app.nExlusive == 1)
        {
            if(app.nUseProxy == 1)
            {
                uCheck = 2;
            }
            else
            {
                uCheck = 1;
            }
            dw = CheckMenuRadioItem(hMenu, 1, 2, uCheck, MF_BYPOSITION);
            dw = GetLastError();
        }
        else
        {
            if(app.nUseProxy == 1)
            {
                CheckMenuItem(hMenu, 2, MF_CHECKED | MF_BYPOSITION);
            }
            if(app.nUseScript == 1)
            {
                CheckMenuItem(hMenu, 1, MF_CHECKED | MF_BYPOSITION);
            }
        }
    }
    if(ad.scriptind > 0)
    {
        RemoveMenu(hScriptMenu, 0, MF_BYPOSITION);
        for(i = 0; i < ad.scriptind; ++i)
        {
            AppendMenu(hScriptMenu, MF_STRING, (MENUACSBASEID+i), ad.scripts[i]);
        }
        CheckMenuRadioItem(hScriptMenu, 0, (ad.scriptind-1), ad.scriptuse, MF_BYPOSITION);
    }
    if(ad.serverind > 0)
    {
        RemoveMenu(hProxyMenu, 0, MF_BYPOSITION);
        for(i = 0; i < ad.serverind; ++i)
        {
            AppendMenu(hProxyMenu, MF_STRING, (MENUPSBASEID+i), ad.servers[i]);
        }
        CheckMenuRadioItem(hProxyMenu, 0, (ad.serverind-1), ad.serveruse, MF_BYPOSITION);
    }
    if(app.nAutoStart == 1)
        CheckMenuItem(hSettingMenu, ID_AUTOSTART, MF_CHECKED | MF_BYCOMMAND);
    if(app.nExlusive == 1)
        CheckMenuItem(hSettingMenu, ID_UPCSE, MF_CHECKED | MF_BYCOMMAND);
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON| TPM_VERNEGANIMATION, pt.x, pt.y, hwnd, 0);
    DestroyMenu(g_hMenu);
    return TRUE;
}

unsigned __stdcall test_response(void *arg)
{
    int index, n;
    long time;
    index = (int)arg;
    while(td.exit != 1)
    {
        WaitForSingleObject(td.hEvents[index], INFINITE);
        if(td.exit == 1) break;
        for(n = index; n < ad.serverind; n += td.count)
        {
            time = send_http(ad.servers[n], "www.google.com"); 
            SendMessage(g_hDlg, WM_UPDATERT, n, time);
        }
        ResetEvent(td.hEvents[index]);
    }
    return 0;
}

BOOL OnCreateThreads(HWND hwnd)
{
    int i;
    td.count = app.nThread - 1;
    if(td.count > ad.serverind)
        td.count = ad.serverind;
    td.hThreads = (HANDLE *)malloc(((sizeof(HANDLE)*td.count)<<1));
    td.hEvents = (HANDLE *)(td.hThreads + td.count);
    td.exit = 0;
    for(i = 0; i < td.count; ++i)
    {
        td.hEvents[i] = (HANDLE)CreateEvent(NULL, TRUE, TRUE, NULL);
        td.hThreads[i] = (HANDLE)_beginthreadex(NULL, 0, test_response, (void *)i, 0, 0);
    }
    return TRUE;
}

BOOL CleanupThreads(HWND hwnd)
{
    int i;
    td.exit = 1;
    for(i = 0; i < td.count; ++i)
    {
        SetEvent(td.hEvents[i]);
    }
    WaitForMultipleObjects(td.count, td.hThreads, TRUE, 100);
    for(i = 0; i < td.count; ++i)
    {
        CloseHandle(td.hEvents[i]);
        CloseHandle(td.hThreads[i]);
    }
    free(td.hThreads);
    return TRUE;
}

BOOL EvokeThreads()
{
    int i;
    for(i = 0; i < td.count; ++i)
    {
        SetEvent(td.hEvents[i]);
    }
    return TRUE;
}


LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int cmd;
    int index;
    RECT rect;
    if(msg== uPostMessage)
    {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        Shell_NotifyIcon(NIM_ADD, &nid);
        return FALSE;
    }
    switch(msg)
    {
    case WM_CREATE:
        OnCreate(hwnd, wParam, lParam);
        PostMessage(hwnd, WM_CREATETHREADS, 0, 0);
        return 0;

    case WM_CREATETHREADS:
        OnCreateThreads(hwnd);
        SetTimer(hwnd, 0, (app.nRefresh * 60 * 1000), NULL);
        return 0;

    case WM_TIMER:
        EvokeThreads();
        return 0;

    case WM_CLOSE:
        GetWindowRect(hwnd, &rect);
        app.xWin = (USHORT)(rect.left);
        app.yWin = (USHORT) (rect.top);
        app.xWidth = (USHORT)(rect.right-rect.left);
        app.yHeight =(USHORT)( rect.bottom-rect.top);
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_SIZE:
        if(wParam == SIZE_MAXIMIZED)
            app.nCmdShow = SW_SHOWMAXIMIZED;
        else
            app.nCmdShow = SW_SHOWNORMAL;
        app.xWidth = LOWORD(lParam);
        app.yHeight = HIWORD(lParam);
        AdjustWindow(hwnd);
        return 0;

    case WM_COMMAND:
        cmd = LOWORD(wParam);
        if(cmd >= MENUACSBASEID && cmd < MENUACSBASEID+ad.scriptind)
        {
            index = cmd - MENUACSBASEID;
            ad.scriptuse = index;
            SendMessage(GetDlgItem(g_hDlg, IDC_COMBO_SCRIPT), CB_SETCURSEL, (WPARAM)index, 0);
            if(app.nUseScript)
            {
                SendMessage(GetDlgItem(g_hDlg, IDC_COMBO_SCRIPT), WM_COMMAND, MAKELPARAM(IDC_COMBO_SCRIPT, CBN_CLOSEUP), 0);
            }
            else
            {
                SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_SCRIPT), BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(g_hDlg, WM_COMMAND, IDC_CHECK_SCRIPT, 0);
            }
        }
        else if(cmd >= MENUPSBASEID && cmd < MENUPSBASEID+ad.serverind)
        {
            index = cmd - MENUPSBASEID;
            ad.serveruse = index;
            SendMessage(GetDlgItem(g_hDlg, IDC_COMBO_PROXY), CB_SETCURSEL, (WPARAM)index, 0);
            if(app.nUseProxy)
            {
                SendMessage(GetDlgItem(g_hDlg, IDC_COMBO_PROXY), WM_COMMAND, MAKELPARAM(IDC_COMBO_PROXY, CBN_CLOSEUP), 0);
            }
            else
            {
                SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_PROXY), BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(g_hDlg, WM_COMMAND, IDC_CHECK_PROXY, 0);
            }
        }
        else{
            switch(cmd)
            {
            case ID_DIRECT_CONNECT:
                if(app.nUseProxy)
                {
                    SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_PROXY), BM_SETCHECK, BST_UNCHECKED, 0);
                    SendMessage(g_hDlg, WM_COMMAND, IDC_CHECK_PROXY, 0);
                }
                if(app.nUseScript)
                {
                    SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_SCRIPT), BM_SETCHECK, BST_UNCHECKED, 0);
                    SendMessage(g_hDlg, WM_COMMAND, IDC_CHECK_SCRIPT, 0);
                }
                break;

            case ID_OPEN_WINPH:
                ShowWindow(hwnd, SW_SHOWNORMAL);
                break;

            case ID_CLOSE_WINPH:
                PostMessage(hwnd, WM_DESTROY, 0, 0);
                break;

            case ID_UPCSE:
                app.nExlusive = app.nExlusive == 1 ? 0 : 1;
                SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_EX), BM_SETCHECK, app.nExlusive, 0);
                break;

            case ID_AUTOSTART:
                app.nAutoStart = app.nAutoStart == 1 ? 0 : 1;
                SendMessage(GetDlgItem(g_hDlg, IDC_CHECK_AUTO), BM_SETCHECK, app.nAutoStart, 0);
                break;
            }
        }
        return 0;

    case WM_TASKBAR:
        {
            switch(lParam)
            {
            case WM_LBUTTONDBLCLK:
                ShowWindow(hwnd, SW_SHOWNORMAL);
                break;
            case WM_LBUTTONDOWN:
            case WM_CONTEXTMENU:
            case WM_RBUTTONDOWN:
                OnCreateMenu(hwnd, wParam);
                break;
            }
        }
        return 0;

    case WM_CHAR:
        return 0;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        DestroyWindow(g_hView);
        DestroyWindow(g_hDlg);
        CleanupThreads(hwnd);
        PostQuitMessage(wParam);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}