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

#include <ws2tcpip.h>
#include <windows.h>
#include "conn.h"
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#if defined(_MSC_VER)
  #if (_MSC_VER >= 1400)
    #pragma  warning(disable: 4996)
  #endif
#endif /* _MSC_VER */

void parse_proxy(wpString proxy, char *server, int * port)
{
    int i, value;
    if(server == 0)
        return;
    value = 0;
    for(i = 0; proxy[i] && proxy[i] != ':'; ++i)
    {
        server[i] = (char)(proxy[i]);
    }
    server[i] = 0;
    for(i += 1; proxy[i]; ++i)
    {
        value = value * 10 + (proxy[i] - '0');
    }
    *port = value;
}

int get_addr(char *host, unsigned long *addrs, int count)
{
    struct addrinfo *result;
    struct addrinfo *ptr;
    struct addrinfo hints;
    int rc, i;
    struct sockaddr_in  *sockaddr_ipv4;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    i = 0;
    rc = getaddrinfo(host, NULL, &hints, &result);
    if(rc == 0)
    {
        for(ptr = result; ptr!=NULL && i < count; ptr=ptr->ai_next, ++i)
        {
            sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
            *(addrs+i) = sockaddr_ipv4->sin_addr.S_un.S_addr;
        }
    }
    return i;
}

long send_http(wpString proxy, const char *url)
{
    char server[256],
         buffer[1024];
    int port,
        recvlen, sendlen,
        addrcnt, i;
    unsigned long time_start, time_end, diff;
    unsigned long addrs[10];
    struct sockaddr_in addr;
    SOCKET skt;
    parse_proxy(proxy, server, &port);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    sendlen = sprintf(buffer, 
        "GET http://%s/ HTTP/1.1\r\nUser-Agent: Mozilla 4.0(Compatiable MSIE 7.0; Windows NT 6.1)\r\nHost: %s\r\nProxy-Connection:Keep-Alive\r\n\r\n", 
        url, url);
    skt = socket(AF_INET, SOCK_STREAM, 0);
    addrcnt = get_addr(server, addrs, 10);
    diff = 0;
    for(i = 0; i < addrcnt; ++i)
    {
        addr.sin_addr.S_un.S_addr = addrs[i];
        if(connect(skt, (struct sockaddr *)&addr, sizeof(addr)))
        {
            continue;
        }
        else
        {
            time_start = GetTickCount();
            send(skt, buffer, sendlen, 0);
            recvlen = recv(skt, buffer, 1024, 0);
            time_end = GetTickCount();
            if(recvlen < 1)
            {
                diff = 0;
                continue;
            }
            else
            {
                diff = time_end - time_start;
                break;
            }
        }
    }
    closesocket(skt);
    return diff;
}
