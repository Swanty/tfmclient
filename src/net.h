/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#ifndef NET_H
#define NET_H
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include "utils.h"

#define DEFAULT_BUF_SIZE	65535
#define BUF_DIVIDE			65535

#pragma pack(push, 1)
typedef struct
{
	uint ip;
	ushort port;
} Address;
#pragma pack(pop)

int recvall(int, char*, int);

string *recvTo(int, string*, char);

int connection(char*, ushort);

int connectAddr(Address);

#endif
