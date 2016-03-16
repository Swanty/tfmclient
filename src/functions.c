/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#include "functions.h"
#include "http.h"

NPError NPN_GetURL(NPP instance, const char* url, const char* window) // TODO: Uncomplete
{
#ifdef DEBUG
	debug("GetURL");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_PostURL(NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file)
{
#ifdef DEBUG
	debug("PostURL");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
#ifdef DEBUG
	debug("RequestRead");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_NewStream(NPP instance, NPMIMEType type, const char* window, NPStream** stream)
{
#ifdef DEBUG
	debug("NewStream");
#endif
	return NPERR_NO_ERROR;
}

int32_t NPN_Write(NPP instance, NPStream* stream, int32_t len, void* buffer)
{
#ifdef DEBUG
	debug("Write");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
#ifdef DEBUG
	debug("DestroyStream");
#endif
	return NPERR_NO_ERROR;
}

void NPN_Status(NPP instance, const char* message)
{
#ifdef DEBUG
	debug("Status");
#endif
}

const char* NPN_UserAgent(NPP instance) { return USER_AGENT; }

uint32_t NPN_MemFlush(uint32_t size)
{
#ifdef DEBUG
	debug("MemFlush");
#endif
	return NPERR_NO_ERROR;
}

void NPN_ReloadPlugins(NPBool reloadPages)
{
#ifdef DEBUG
	debug("ReloadPlugins");
#endif
}

void *NPN_GetJavaEnv(void)
{
#ifdef DEBUG
	debug("GetJavaEnv");
#endif
	return NPERR_NO_ERROR;
}

void *NPN_GetJavaPeer(NPP instance)
{
#ifdef DEBUG
	debug("GetJavaPeer");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_GetValue(NPP instance, NPNVariable variable, void *ret_value)
{
#ifdef DEBUG
	debug("GetValue %lld", variable);
#endif
	switch (variable)
	{
		//Unix and solaris fix
		case NPNVToolkit:
			*((int*)ret_value)= NPNVGtk2;
			break;
		//case NPNVSupportsWindowless:
		//case NPNVprivateModeBool:
		case NPNVSupportsXEmbedBool:
		case NPNVnetscapeWindow:
			*((int*)ret_value)= TRUE;
			break;
		default:
			*((int*)ret_value)= FALSE;
			break;//return NPERR_GENERIC_ERROR;
	}
	return NPERR_NO_ERROR;
}

NPError NPN_SetValue(NPP instance, NPPVariable variable, void *value)
{
#ifdef DEBUG
	debug("SetValue");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_GetURLNotify(NPP instance, const char* url, const char* window, void* notifyData)
{
#ifdef DEBUG
	//debug("GetURLNotify %p URL: %s, window: %s", instance, url, window);
#endif
	if (window && ((strlen(window)==4 && memcmp("_new", window, 4)==0) || (strlen(window)==6 && memcmp("_blank", window, 6)==0)))
	{
		char *cmd = (char*)malloc(strlen(url)+strlen(BLANK_LINK_CMD)+2);
		int res;
		sprintf(cmd, "%s %s", BLANK_LINK_CMD, url);
		res = system(cmd);
		free(cmd);
		pFuncs.urlnotify(instance, url, res==0?NPRES_DONE:NPRES_NETWORK_ERR, notifyData);
	}
	else if (memcmp(url, "javascript:", 11)==0)
	{
		NPStream s;
		uint16_t stype;
		int res;
		memset(&s, 0, sizeof(NPStream));
		s.url = strdup(url);
		res = (pFuncs.newstream(instance, "application/x-shockwave-flash", &s, 0, &stype)==NPERR_NO_ERROR);
		if (res)
		{
			int pos = 0;
			int size;
			char buf[256];
			sprintf(buf, "%lX__flashplugin_unique__", (intptr_t)instance);
			size = (int)strlen(buf);
			s.end = size;
			while(pos < size)
			{
				int len = pFuncs.writeready(instance, &s);
				if (len <= 0)
					break;
				if (len > size - pos)
					len = size - pos;
				len = pFuncs.write(instance, &s, pos, len, buf+pos);
				if (len <= 0)
					break;
				pos += len;
			}
			res = (pos==size);
		}
		pFuncs.urlnotify(instance, url, res?NPRES_DONE:NPRES_NETWORK_ERR, notifyData);
		pFuncs.destroystream(instance, &s, NPRES_DONE);
		free((char*)s.url);
	}
	else if (memcmp(url, "http:", 5)==0)
	{
		int sock = makeGetRequest(url);
		if (sock)
		{
			NPStream s;
			uint16_t stype;
			int res;
			char *headers = (char*)malloc(1), *contentType = "application/x-shockwave-flash";
			httpPack pack;
			memset(&s, 0, sizeof(NPStream));
			s.url = strdup(url);
			s.notifyData = notifyData;
			headers[0] = 0;
			memset(&pack, 0, sizeof(httpPack));
			recvHttpHeaders(sock, &pack);
			serverString *startString = (serverString*)pack.startString;
			for (int i = 0; i < pack.headerNum; i++)
			{
				int len = strlen(pack.headers[i].header)+strlen(pack.headers[i].value)+5;
				char *temp = (char*)malloc(len);
				sprintf(temp, "%s: %s\r\n", pack.headers[i].header, pack.headers[i].value);
				headers = (char*)realloc(headers, strlen(headers)+len);
				strcat(headers, temp);
				free(temp);
				if (strlen(pack.headers[i].header)==12 && strcmp(pack.headers[i].header, "Content-Type")==0) contentType = pack.headers[i].value;
			}
			headers = (char*)realloc(headers, strlen(headers)+2);
			strcat(headers, "\n");
			s.headers = headers;
			res = (pFuncs.newstream(instance, contentType, &s, 0, &stype)==NPERR_NO_ERROR);
			if (res)
			{
				int pos = 0, size = startString->body.length;
				char buf[BUF_DIVIDE];
				s.end = size;
				while(pos < size)
				{
					int len = pFuncs.writeready(instance, &s);
					if (len <= 0)
						break;
					putchar('\r');
					len = recv(sock, buf, BUF_DIVIDE, 0);
					if (len > size - pos)
						len = size - pos;
					pFuncs.write(instance, &s, pos, len, buf);
					if (len <= 0)
						break;
					pos += len;
				}
				res = (pos==size);
			}
			pFuncs.urlnotify(instance, url, res?NPRES_DONE:NPRES_NETWORK_ERR, notifyData);
			pFuncs.destroystream(instance, &s, NPRES_DONE);
			free((char*)s.url);
		}
	}
	else
	{
		pFuncs.urlnotify(instance, url, NPRES_NETWORK_ERR, notifyData);
	}
	return NPERR_NO_ERROR;
}

NPError NPN_PostURLNotify(NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file, void* notifyData)
{
#ifdef DEBUG
	debug("PostURLNotify");
#endif
	return NPERR_NO_ERROR;
}

void NPN_InvalidateRect(NPP instance, NPRect *rect)
{
#ifdef DEBUG
	debug("InvalidateRect");
#endif
}

void NPN_InvalidateRegion(NPP instance, NPRegion region)
{
#ifdef DEBUG
	debug("InvalidateRegion");
#endif
}

void NPN_ForceRedraw(NPP instance)
{
#ifdef DEBUG
	debug("ForceRedraw");
#endif
}

NPIdentifier NPN_GetStringIdentifier(const NPUTF8* name)
{
#ifdef DEBUG
	debug("GetStringIdentifier");
#endif
	return NPERR_NO_ERROR;
}

void NPN_GetStringIdentifiers(const NPUTF8** names, int32_t nameCount, NPIdentifier* identifiers)
{
#ifdef DEBUG
	debug("GetStringIdentifiers");
#endif
}

NPIdentifier NPN_GetIntIdentifier(int32_t intid)
{
#ifdef DEBUG
	debug("GetIntIdentifier");
#endif
	return NPERR_NO_ERROR;
}

bool NPN_IdentifierIsString(NPIdentifier identifier)
{
#ifdef DEBUG
	debug("IdentifierIsString");
#endif
	return TRUE;
}

NPUTF8* NPN_UTF8FromIdentifier(NPIdentifier identifier)
{
#ifdef DEBUG
	debug("UTF8FromIdentifier");
#endif
	return NPERR_NO_ERROR;
}

int32_t NPN_IntFromIdentifier(NPIdentifier identifier)
{
#ifdef DEBUG
	debug("IntFromIdentifier");
#endif
	return NPERR_NO_ERROR;
}

NPObject *NPN_CreateObject(NPP npp, NPClass *aClass) // TODO: Copy
{
#ifdef DEBUG
	debug("CreateObject C");
#endif
	NPObject *o;
	if (aClass->allocate)
		o = aClass->allocate(npp,aClass);
	else
		o = (NPObject*)malloc(sizeof(NPObject));
	o->_class = aClass;
	o->referenceCount = 1;
	return o;
}

NPObject *NPN_RetainObject(NPObject *obj) // TODO: Copy
{
#ifdef DEBUG
	debug("RetainObject C");
#endif
	if (obj) obj->referenceCount++;
	return obj;
}

void NPN_ReleaseObject(NPObject *obj)
{
#ifdef DEBUG
	debug("ReleaseObject");
#endif
}

bool NPN_Invoke(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
#ifdef DEBUG
	debug("Invoke");
#endif
	return TRUE;
}

bool NPN_InvokeDefault(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
#ifdef DEBUG
	debug("InvokeDefault");
#endif
	return TRUE;
}

bool NPN_Evaluate(NPP npp, NPObject *obj, NPString *script, NPVariant *result)
{
#ifdef DEBUG
	debug("Evaluate");
#endif
	return TRUE;
}

bool NPN_GetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result)
{
#ifdef DEBUG
	debug("GetProperty");
#endif
	return TRUE;
}

bool NPN_SetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value)
{
#ifdef DEBUG
	debug("SetProperty");
#endif
	return TRUE;
}

bool NPN_RemoveProperty(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
#ifdef DEBUG
	debug("RemoveProperty");
#endif
	return TRUE;
}

bool NPN_HasProperty(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
#ifdef DEBUG
	debug("HasProperty");
#endif
	return TRUE;
}

bool NPN_HasMethod(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
#ifdef DEBUG
	debug("HasMethod");
#endif
	return TRUE;
}

void NPN_ReleaseVariantValue(NPVariant *variant)
{
#ifdef DEBUG
	debug("ReleaseVariantValue");
#endif
}

void NPN_SetException(NPObject *obj, const NPUTF8 *message)
{
#ifdef DEBUG
	debug("SetException");
#endif
}

void NPN_PushPopupsEnabledState(NPP npp, NPBool enabled)
{
#ifdef DEBUG
	debug("PushPopupsEnabledState");
#endif
}

void NPN_PopPopupsEnabledState(NPP npp)
{
#ifdef DEBUG
	debug("PopPopupsEnabledState");
#endif
}

bool NPN_Enumerate(NPP npp, NPObject *obj, NPIdentifier **identifier, uint32_t *count)
{
#ifdef DEBUG
	debug("Enumerate");
#endif
	return TRUE;
}

void NPN_PluginThreadAsyncCall(NPP instance, void func(void *), void *userData)
{
#ifdef DEBUG
	debug("PluginThreadAsyncCall");
#endif
}

bool NPN_Construct(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
#ifdef DEBUG
	debug("Construct");
#endif
	return TRUE;
}

NPError NPN_GetValueForURL(NPP npp, NPNURLVariable variable, const char *url, char **value, uint32_t *len)
{
#ifdef DEBUG
	debug("GetValueForURL");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_SetValueForURL(NPP npp, NPNURLVariable variable, const char *url, const char *value, uint32_t len)
{
#ifdef DEBUG
	debug("SetValueForURL");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_GetAuthenticationInfo(NPP npp, const char *protocol, const char *host, int32_t port, const char *scheme, const char *realm, char **username, uint32_t *ulen, char **password, uint32_t *plen)
{
#ifdef DEBUG
	debug("GetAuthenticationInfo");
#endif
	return NPERR_NO_ERROR;
}

uint32_t NPN_ScheduleTimer(NPP instance, uint32_t interval, NPBool repeat, void timerFunc(NPP npp, uint32_t timerID))
{
#ifdef DEBUG
	debug("ScheduleTimer");
#endif
	return NPERR_NO_ERROR;
}

void NPN_UnscheduleTimer(NPP instance, uint32_t timerID)
{
#ifdef DEBUG
	debug("UnscheduleTimer");
#endif
}

NPError NPN_PopUpContextMenu(NPP instance, NPMenu* menu)
{
#ifdef DEBUG
	debug("PopUpContextMenu");
#endif
	return NPERR_NO_ERROR;
}

NPBool NPN_ConvertPoint(NPP instance, double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace)
{
#ifdef DEBUG
	debug("ConvertPoint");
#endif
	return TRUE;
}

NPBool NPN_HandleEvent(NPP instance, void *event, NPBool handled)
{
#ifdef DEBUG
	debug("HandleEvent");
#endif
	return TRUE;
}

NPBool NPN_UnfocusInstance(NPP instance, NPFocusDirection direction)
{
#ifdef DEBUG
	debug("UnfocusInstance");
#endif
	return TRUE;
}

void NPN_URLRedirectResponse(NPP instance, void* notifyData, NPBool allow)
{
#ifdef DEBUG
	debug("URLRedirectResponse");
#endif
}

NPError NPN_InitAsyncSurface(NPP instance, NPSize *size, NPImageFormat format, void *initData, NPAsyncSurface *surface)
{
#ifdef DEBUG
	debug("InitAsyncSurface");
#endif
	return NPERR_NO_ERROR;
}

NPError NPN_FinalizeAsyncSurface(NPP instance, NPAsyncSurface *surface)
{
#ifdef DEBUG
	debug("FinalizeAsyncSurface");
#endif
	return NPERR_NO_ERROR;
}

void NPN_SetCurrentAsyncSurface(NPP instance, NPAsyncSurface *surface, NPRect *changed)
{
#ifdef DEBUG
	debug("SetCurrentAsyncSurface");
#endif
}
