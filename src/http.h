/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#ifndef HTTP_H
#define HTTP_H
#include "utils.h"
#include "net.h"

#pragma pack(push, 1)
typedef struct
{
	char *header;
	char *value;
} httpHeader;

typedef httpHeader httpVar;

typedef struct
{
	void *startString;
	int headerNum;
	httpHeader *headers;
} httpPack;

typedef enum
{
	GET,
	POST
} httpMethod;

typedef struct
{
	httpMethod method;
	char *URI;
	int varNum;
	httpVar *vars;
} clientString;

typedef struct
{
	ushort code;
	char *status;
	string body;
} serverString;
#pragma pack(pop)

int makeGetRequest(const char*);

int sendHttpPack(int, httpPack*);

httpPack *recvHttpHeaders(int, httpPack*);

httpPack *recvHttpPack(int, httpPack*);

httpPack *addHTTPHeader(httpPack*, char*, char*);

httpPack *freeHTTPPack(httpPack*);

#endif
