/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#include "net.h"
#include <sys/fcntl.h>

int recvall(int sock, char *buf, int len)
{
	int l = 0, n;
	char *b = NULL;
	while (l < len)
	{
		b = (char*)realloc(b, len-l);
		n = recv(sock, b, len-l, 0);
		if (n==0) break;
		memcpy(buf+l, b, n);
		l += n;
	}
	if (b!=NULL) free(b);
	return l;
}

string *recvTo(int sock, string *buf, char c)
{
	int l = 0, n;
	char b[1] = "";
	while (b[0]!=c)
	{
		n = recv(sock, b, 1, 0);
		if (n==0) break;
		buf->string = (char*)realloc(buf->string, l+2);
		buf->string[l++] = b[0];
	}
	buf->string[l] = 0;
	buf->length = l;
	return buf;
}

int connection(char *host, ushort port)
{
	Address addr;
	struct hostent *hp;
	if ((hp = gethostbyname(host))==NULL) return -1;
	memcpy(&addr.ip, hp->h_addr, 4);
	addr.port = port;
	return connectAddr(addr);
}

int connectAddr(Address addr)
{
	int sock;
	struct sockaddr_in serv_addr;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) perr("cannot create socket");
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = addr.ip;
	serv_addr.sin_port = htons(addr.port);
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) return -1;
	fcntl(sock, F_SETFL, 1);
	return sock;
}
