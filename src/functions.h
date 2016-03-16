/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <gtk/gtk.h>
#include "npapi/npfunctions.h"
#include <sys/ioctl.h>

#define BLANK_LINK_CMD	"xdg-open"

NPPluginFuncs pFuncs;
NPNetscapeFuncs bFuncs;

NPError NPN_GetURL(NPP, const char*, const char*);

NPError NPN_PostURL(NPP, const char*, const char*, uint32_t, const char*, NPBool);

NPError NPN_RequestRead(NPStream*, NPByteRange*);

NPError NPN_NewStream(NPP, NPMIMEType, const char*, NPStream**);

int32_t NPN_Write(NPP, NPStream*, int32_t, void*);

NPError NPN_DestroyStream(NPP, NPStream*, NPReason);

void NPN_Status(NPP, const char*);

const char *NPN_UserAgent(NPP);

uint32_t NPN_MemFlush(uint32_t);

void NPN_ReloadPlugins(NPBool);

void *NPN_GetJavaEnv(void);

void *NPN_GetJavaPeer(NPP);

NPError NPN_GetValue(NPP, NPNVariable, void*);

NPError NPN_SetValue(NPP, NPPVariable, void*);

NPError NPN_GetURLNotify(NPP, const char*, const char*, void*);

NPError NPN_PostURLNotify(NPP, const char*, const char*, uint32_t, const char*, NPBool, void*);

void NPN_InvalidateRect(NPP, NPRect*);

void NPN_InvalidateRegion(NPP, NPRegion);

void NPN_ForceRedraw(NPP);

NPIdentifier NPN_GetStringIdentifier(const NPUTF8*);

void NPN_GetStringIdentifiers(const NPUTF8**, int32_t, NPIdentifier*);

NPIdentifier NPN_GetIntIdentifier(int32_t);

bool NPN_IdentifierIsString(NPIdentifier);

NPUTF8 *NPN_UTF8FromIdentifier(NPIdentifier);

int32_t NPN_IntFromIdentifier(NPIdentifier);

NPObject *NPN_CreateObject(NPP, NPClass*);

NPObject *NPN_RetainObject(NPObject*);

void NPN_ReleaseObject(NPObject*);

bool NPN_Invoke(NPP, NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant*);

bool NPN_InvokeDefault(NPP, NPObject*, const NPVariant*, uint32_t, NPVariant*);

bool NPN_Evaluate(NPP, NPObject*, NPString*, NPVariant*);

bool NPN_GetProperty(NPP, NPObject*, NPIdentifier, NPVariant*);

bool NPN_SetProperty(NPP, NPObject*, NPIdentifier, const NPVariant*);

bool NPN_RemoveProperty(NPP, NPObject*, NPIdentifier);

bool NPN_HasProperty(NPP, NPObject*, NPIdentifier);

bool NPN_HasMethod(NPP, NPObject*, NPIdentifier);

void NPN_ReleaseVariantValue(NPVariant*);

void NPN_SetException(NPObject*, const NPUTF8*);

void NPN_PushPopupsEnabledState(NPP, NPBool);

void NPN_PopPopupsEnabledState(NPP);

bool NPN_Enumerate(NPP, NPObject*, NPIdentifier**, uint32_t*);

void NPN_PluginThreadAsyncCall(NPP, void func(void*), void*);

bool NPN_Construct(NPP, NPObject*, const NPVariant*, uint32_t, NPVariant*);

NPError NPN_GetValueForURL(NPP, NPNURLVariable, const char*, char**, uint32_t*);

NPError NPN_SetValueForURL(NPP, NPNURLVariable, const char*, const char*, uint32_t);

NPError NPN_GetAuthenticationInfo(NPP, const char*, const char*, int32_t, const char*, const char*, char**, uint32_t*, char**, uint32_t*);

uint32_t NPN_ScheduleTimer(NPP, uint32_t, NPBool, void timerFunc(NPP, uint32_t));

void NPN_UnscheduleTimer(NPP, uint32_t);

NPError NPN_PopUpContextMenu(NPP, NPMenu*);

NPBool NPN_ConvertPoint(NPP, double, double, NPCoordinateSpace, double*, double*, NPCoordinateSpace);

NPBool NPN_HandleEvent(NPP, void*, NPBool);

NPBool NPN_UnfocusInstance(NPP, NPFocusDirection);

void NPN_URLRedirectResponse(NPP, void*, NPBool);

NPError NPN_InitAsyncSurface(NPP, NPSize*, NPImageFormat, void*, NPAsyncSurface*);

NPError NPN_FinalizeAsyncSurface(NPP, NPAsyncSurface*);

void NPN_SetCurrentAsyncSurface(NPP, NPAsyncSurface*, NPRect*);

#endif
