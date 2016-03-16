/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include "net.h"

#ifdef DEBUG
void debug(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	printf("[DEBG] ");
	vprintf(msg, args);
	printf("\n");
	va_end(args);
}
#endif

void create_thread(void *(*function) (void*), void *arg)
{
#ifdef _WIN32
	CreateThread(NULL, 0, function, arg, 0, NULL);
#elif __unix__
	pthread_t thread;
	pthread_create(&thread, NULL, function, arg);
	pthread_detach(thread);
#endif
}

void perr(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "%s: ", pname);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(EXIT_FAILURE);
}

string *freeString(string *str)
{
	if (str->length > 0 && str->string!=NULL) free(str->string);
	str->length = 0;
	str->string = NULL;
	return str;
}
