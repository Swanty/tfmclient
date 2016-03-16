/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#include "http.h"

int makeGetRequest(const char *argUrl)
{
	int i, sock = 0;
	char *url = strdup(argUrl), s = 0, *host = NULL;
	httpPack pack;
	clientString *startString;

	memset(&pack, 0, sizeof(httpPack));

	pack.startString = malloc(sizeof(clientString));
	startString = (clientString*)pack.startString;
	memset(startString, 0, sizeof(clientString));

	startString->method = GET;
	// Get URI and host
	for (i = 0; i < strlen(url); i++)
	{
		switch (s)
		{
			case 0: // Skip http://, ftp://, etc.
				if (url[i]==':') s++;
				break;
			case 1: // Get host first char pointer
				if (url[i]!='/')
				{
					host = url+i;
					s++;
				}
				break;
			case 2: // Cut host from URL, and get URI
				if (url[i]=='/')
				{
					char *buf = (char*)malloc(url+i-host+1);
					strncpy(buf, host, url+i-host);
					buf[url+i-host] = 0;
					addHTTPHeader(&pack, "Host", buf);
					sock = connection(buf, 80);
					free(buf);
					startString->URI = url+i;
					s++;
				}
				break;
		}
		if (s==3) break;
	}
	// Fill headers
	addHTTPHeader(&pack, "User-Agent", USER_AGENT);
	addHTTPHeader(&pack, "Accept", "*/*");

	if (sock) sendHttpPack(sock, &pack);
	// Cleaning
	free(url);
	freeHTTPPack(&pack);
	return sock;
}

int sendHttpPack(int sock, httpPack *pack)
{
	int i, res;
	char *buf = (char*)malloc(1), *vars = (char*)malloc(1);
	clientString *startString = (clientString*)pack->startString;
	buf[0] = vars[0] = 0;
	// Generate variables
	for (i = 0; i < startString->varNum; i++)
	{
		char *var = startString->vars[i].header;
		char *value = startString->vars[i].value;
		vars = (char*)realloc(vars, strlen(vars)+strlen(var)+strlen(value)+2+(i!=0&&i!=startString->varNum));
		sprintf(vars, "%s%s%s=%s", vars, i!=0?"&":"", var, value);
	}
	// Append headers
	for (i = 0; i < pack->headerNum; i++)
	{
		char *header = pack->headers[i].header;
		char *value = pack->headers[i].value;
		int len = strlen(header)+strlen(value)+5;
		char *temp = (char*)malloc(len);
		sprintf(temp, "%s: %s\r\n", header, value);
		buf = (char*)realloc(buf, strlen(buf)+len);
		strcat(buf, temp);
		free(temp);
	}
	char *temp = strdup(buf);
	// Generate full request with methods
	switch (startString->method)
	{
		case POST:
			buf = (char*)realloc(buf, strlen(buf)+strlen(vars)+strlen(startString->URI)+18);
			sprintf(buf, "POST %s HTTP/1.1\r\n%s\r\n%s", startString->URI, temp, vars);
			break;
		default:
			buf = (char*)realloc(buf, strlen(buf)+strlen(vars)+strlen(startString->URI)+18+(strlen(vars)>0));
			sprintf(buf, "GET %s%s%s HTTP/1.1\r\n%s\r\n", startString->URI, strlen(vars)>0?"?":"",vars, temp);
			break;
	}
	free(temp);
	free(vars);
	res = send(sock, buf, strlen(buf), 0);
	free(buf);
	return res;
}

httpPack *recvHttpHeaders(int sock, httpPack *pack)
{
	string buf = { 0, (char*)malloc(1) };

	if (pack->startString==NULL) pack->startString = malloc(sizeof(serverString));
	serverString *startString = (serverString*)pack->startString;
	memset(startString, 0, sizeof(serverString));
	// Skip HTTP/1.1
	recvTo(sock, &buf, ' ');
	// Get status code
	recvTo(sock, &buf, ' ');
	startString->code = atoi(buf.string);
	// Get status message
	recvTo(sock, &buf, '\n');
	buf.string[buf.length-2] = 0;
	startString->status = strdup(buf.string);
	// Getting headers
	while ((recvTo(sock, &buf, '\n'))->length!=2) // To \r\n
	{
		int i = 0;
		char *header = buf.string;
		char *value;
		buf.string[buf.length-2] = 0;
		while (buf.string[i]!=':' && i < strlen(buf.string)) i++; // Get header name
		buf.string[i] = 0;
		value = buf.string+i+2; // value
		addHTTPHeader(pack, header, value);
		if (strlen(header)==14 && strcmp(header, "Content-Length")==0) startString->body.length = atoi(value); // Get content length
	}
	free(buf.string);
	return pack;
}

httpPack *recvHttpBody(int sock, httpPack *pack)
{
	int len;
	serverString *startString = (serverString*)pack->startString;
	// Get body
	if (!startString->body.length) startString->body.length = DEFAULT_BUF_SIZE;
	startString->body.string = (char*)malloc(startString->body.length);
	len = recvall(sock, startString->body.string, startString->body.length);
	if (startString->body.length!=len) startString->body.string = (char*)realloc(startString->body.string, len);
	return pack;
}

httpPack *recvHttpPack(int sock, httpPack *pack)
{
	recvHttpHeaders(sock, pack);
	recvHttpBody(sock, pack);
	return pack;
}

httpPack *addHTTPHeader(httpPack *pack, char *header, char *value)
{
	pack->headers = (httpHeader*)realloc(pack->headers, ++pack->headerNum*sizeof(httpHeader));
	pack->headers[pack->headerNum-1].header	= strdup(header);
	pack->headers[pack->headerNum-1].value	= strdup(value);
	return pack;
}

httpPack *freeHTTPPack(httpPack *pack)
{
	int i;
	for (i = 0; i < pack->headerNum; i++)
	{
		free(pack->headers[i].header);
		free(pack->headers[i].value);
	}
	free(pack->headers);
	if (pack->startString!=NULL) free(pack->startString);
	return pack;
}
