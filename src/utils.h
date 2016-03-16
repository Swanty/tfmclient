/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#ifndef UTILS_H
#define UTILS_H
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "tfmcheat.h"

//#define USER_AGENT		"Mozilla/5.0 (X11; Linux x86_64; rv:38.0) Gecko/20100101 Firefox/38.0"
#define USER_AGENT		NAME " Linux Client " VERSION

//#define DEBUG // For NPAPI debug output

#pragma pack(push, 1)
typedef struct
{
	int length;
	char *string;
} string;
#pragma pack(pop)

typedef unsigned long int ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef uchar byte;

#ifdef DEBUG
void debug(const char*, ...);
#endif

void create_thread(void *(*function) (void*), void*);

char *pname;

void perr(const char*, ...);

#endif
