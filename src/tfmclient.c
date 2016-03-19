/*
 * Copyright (c) editor700
 * (c) 2016 <editor700@gmail.com>
 */

#include <dlfcn.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include "functions.h"
#include "utils.h"
#include "npapi/npapi.h"

#define WIN_WIDTH		800
#define WIN_HEIGHT		600

#define WIN_POSITION	GTK_WIN_POS_CENTER

extern char loader_data[]      asm("_binary_res_TransformiceChargeur_swf_start");
extern char loader_data_end[]  asm("_binary_res_TransformiceChargeur_swf_end");

// Windows
GtkWidget *mainWindow;
NPWindow *npwin;

bool fullscreen;

NPP instance; // Plugin instance

char *iconPaths[] =
{
	"/usr/share/pixmaps",
	"",
	"res"
}, *iconNames[] =
{
	"transformice.png",
}, *flashPlayerNames[] =
{
#ifdef FLASH_SO_NAME
	FLASH_SO_NAME,
#endif
	"libflashplayer.so",
	"flashplugin-alternative.so"
}, *flashPlayerPaths[] =
{
#ifdef FLASH_SO_LOCATION
	FLASH_SO_LOCATION,
#endif
	"/usr/lib/mozilla/plugins",
	"/usr/lib/nsbrowser/plugins",
	"~/.mozilla/plugins",
	""
}, *flashPlayerArg;

char *find_file(char *const *names, char *const *paths, int name_col, int path_col)
{
	int i;
	for (i = 0; i < path_col; i++)
	{
		int n;
		for (n = 0; n < name_col; n++)
		{
			char *path = (char*)malloc(strlen(names[n])+strlen(paths[i])+2);
			FILE *f;
			sprintf(path, "%s/%s", paths[i], names[n]);
			if ((f = fopen(path, "r"))!=NULL)
			{
				fclose(f);
				return path;
			}
			free(path);
		}
	}
	return NULL;
}

GdkPixbuf *create_pixbuf()
{
	char *path = find_file(iconNames, iconPaths, sizeof(iconNames)/sizeof(char*), sizeof(iconPaths)/sizeof(char*));
	if (path)
	{
		GdkPixbuf *pixbuf;
		GError *error = NULL;
		pixbuf = gdk_pixbuf_new_from_file(path, &error);
		free(path);
		return pixbuf;
	}
	return NULL;
}

void *loadSymbol(void *handle, const char *sym)
{
	char *error;
	void *res = dlsym(handle, sym);
	if ((error = dlerror())!=NULL) perr("cannot load symbol '%s': %s", sym, error);
	return res;
}

static void initBrowserFuncs(NPNetscapeFuncs *browserFuncs)
{
	browserFuncs->size = sizeof(NPNetscapeFuncs);
	browserFuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;

	browserFuncs->geturl =	NPN_GetURL;
	browserFuncs->posturl =	NPN_PostURL;
	browserFuncs->requestread =	NPN_RequestRead;
	browserFuncs->newstream =	NPN_NewStream;
	browserFuncs->write =	NPN_Write;
	browserFuncs->destroystream =	NPN_DestroyStream;
	browserFuncs->status =	NPN_Status;
	browserFuncs->uagent =	NPN_UserAgent;
	browserFuncs->memalloc =	(void*)malloc;
	browserFuncs->memfree =	free;
	browserFuncs->memflush =	NPN_MemFlush;
	browserFuncs->reloadplugins =	NPN_ReloadPlugins;
	browserFuncs->getJavaEnv =	NPN_GetJavaEnv;
	browserFuncs->getJavaPeer =	NPN_GetJavaPeer;
	browserFuncs->geturlnotify =	NPN_GetURLNotify;
	browserFuncs->posturlnotify =	NPN_PostURLNotify;
	browserFuncs->getvalue =	NPN_GetValue;
	browserFuncs->setvalue =	NPN_SetValue;
	browserFuncs->invalidaterect =	NPN_InvalidateRect;
	browserFuncs->invalidateregion =	NPN_InvalidateRegion;
	browserFuncs->forceredraw =	NPN_ForceRedraw;
	browserFuncs->getstringidentifier =	NPN_GetStringIdentifier;
	browserFuncs->getstringidentifiers =	NPN_GetStringIdentifiers;
	browserFuncs->getintidentifier =	NPN_GetIntIdentifier;
	browserFuncs->identifierisstring =	NPN_IdentifierIsString;
	browserFuncs->utf8fromidentifier =	NPN_UTF8FromIdentifier;
	browserFuncs->intfromidentifier =	NPN_IntFromIdentifier;
	browserFuncs->createobject =	NPN_CreateObject;
	browserFuncs->retainobject =	NPN_RetainObject;
	browserFuncs->releaseobject =	NPN_ReleaseObject;
	browserFuncs->invoke =	NPN_Invoke;
	browserFuncs->invokeDefault =	NPN_InvokeDefault;
	browserFuncs->evaluate =	NPN_Evaluate;
	browserFuncs->getproperty =	NPN_GetProperty;
	browserFuncs->setproperty =	NPN_SetProperty;
	browserFuncs->removeproperty =	NPN_RemoveProperty;
	browserFuncs->hasproperty =	NPN_HasProperty;
	browserFuncs->hasmethod =	NPN_HasMethod;
	browserFuncs->releasevariantvalue =	NPN_ReleaseVariantValue;
	browserFuncs->setexception =	NPN_SetException;
	browserFuncs->pushpopupsenabledstate =	NPN_PushPopupsEnabledState;
	browserFuncs->poppopupsenabledstate =	NPN_PopPopupsEnabledState;
	browserFuncs->enumerate =	NPN_Enumerate;
	browserFuncs->pluginthreadasynccall =	NPN_PluginThreadAsyncCall;
	browserFuncs->construct =	NPN_Construct;
	browserFuncs->getvalueforurl =	NPN_GetValueForURL;
	browserFuncs->setvalueforurl =	NPN_SetValueForURL;
	browserFuncs->getauthenticationinfo =	NPN_GetAuthenticationInfo;
	browserFuncs->scheduletimer =	NPN_ScheduleTimer;
	browserFuncs->unscheduletimer =	NPN_UnscheduleTimer;
	browserFuncs->popupcontextmenu =	NPN_PopUpContextMenu;
	browserFuncs->convertpoint =	NPN_ConvertPoint;
	browserFuncs->handleevent =	NPN_HandleEvent;
	browserFuncs->unfocusinstance =	NPN_UnfocusInstance;
	browserFuncs->urlredirectresponse =	NPN_URLRedirectResponse;
	browserFuncs->initasyncsurface =	NPN_InitAsyncSurface;
	browserFuncs->finalizeasyncsurface =	NPN_FinalizeAsyncSurface;
	browserFuncs->setcurrentasyncsurface =	NPN_SetCurrentAsyncSurface;
}

// GTK+ Events
static void destroy(GtkWidget *widget, gpointer data) { gtk_main_quit(); }

void winChanged(GtkWindow *window, GdkEventConfigure *e, gpointer data)
{
	npwin->width = e->width;
	npwin->height = e->height;
	pFuncs.setwindow(instance, npwin);
}

void sendKeyEvent(bool pressed, uint key, uint mask, uint time)
{
	XEvent evt;
	XKeyEvent *xKey = &evt.xkey;
	memset(&evt, 0, sizeof(XEvent));
	evt.xany.display = GDK_DISPLAY();
	evt.xany.type = 2 + !pressed;
	xKey->root = GDK_ROOT_WINDOW();
	xKey->time = time;
	xKey->state = mask;
	xKey->keycode = key;

	xKey->same_screen = TRUE;
	pFuncs.event(instance, &evt);
}

gboolean keyPressed(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval==GDK_KEY_F11)
	{
		void (*f)(GtkWindow*) = fullscreen?gtk_window_fullscreen:gtk_window_unfullscreen;
		f((GtkWindow*)mainWindow);
		fullscreen = !fullscreen;
	}
	else sendKeyEvent(TRUE, event->hardware_keycode, event->state, event->time);
	return TRUE; 
}

gboolean keyReleased(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (event->keyval!=GDK_KEY_F11) sendKeyEvent(FALSE, event->hardware_keycode, event->state, event->time);
	return TRUE;
}

static gboolean plug_removed_cb (GtkWidget *widget, gpointer data) { return TRUE; }

void print_version()
{
	printf("%s %s by %s\n", NAME, VERSION, AUTHOR);
}

void print_help()
{
	print_version();
	printf("Usage: %s [-vhf?] [--version] [--help] [--flash /flash/player/path.so]\n\nOptions:\n", pname);
	printf("  -h, -?, --help    - this help\n");
	printf("  -v, --version     - print version and exit\n");
	printf("  -f, --flash file  - try to use file as Flash Player library\n");
	printf("\n");
}

#define CHK_ARG(arg, type, long, short)	(type==1?*arg==short:strcmp(long, arg)==0)

void parse_args(int argc, char *argv[])
{
	int i;
	pname = argv[0];
	for (i = 1; i < argc; i++)
	{
		char *p = argv[i];
		byte t = 0;
		while (*p=='-')
		{
			t++;
			p = p+1;
		}
		if (t < 1) perr("wrong argument '%s'", argv[i]);
		while (TRUE)
		{
			if (!*p) break;
			if (CHK_ARG(p, t, "version", 'v'))
			{
				print_version();
				exit(0);
			}
			else if (CHK_ARG(p, t, "help", 'h') || (t==1 && *p=='?'))
			{
				print_help();
				exit(0);
			}
			else if (CHK_ARG(p, t, "flash", 'f'))
				if (i+1 < argc) flashPlayerArg = argv[++i];
				else perr("no flash player path for '%s' argument", argv[i]);
			else
			{
				perr("unknown argument '%s'", argv[i]);
				if (!*p) break;
				exit(0);
			}
			if (t==1) p = p+1;
			else break;
		}
	}
}

int main(int argc, char *argv[])
{
	void *flashPlugin = NULL;
	NPError (*NP_Initialize)(NPNetscapeFuncs* bFuncs, NPPluginFuncs* pFuncs);
	NPError (*NP_Shutdown)();
	NPSavedData *savedData;
	NPP_t *instancep;

	NPSetWindowCallbackStruct *ws_info = NULL;
	GtkWidget *socketWidget = gtk_socket_new();
	gpointer user_data = NULL;
	GtkContainer *container;
	GtkAllocation new_allocation;
	GdkNativeWindow ww;
	GdkWindow *w;

	NPStream *stream;
	uint16_t stype;
	NPObject object;
	
	GdkPixbuf *tfmIcon;

	char *xargv[]= {"allowscriptaccess", "name", "quality", "wmode", "allowFullScreen", "width", "height", "scale"},
		*xargm[]= {"always", "Transformice", "best", "direct", "true", "800", "600","exactfit"}, *flashLocation;

	pname = argv[0];
	flashPlayerArg = NULL;

	parse_args(argc, argv);

	gtk_init (&argc, &argv);
	// GTK+ window
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainWindow), "Transformice");
	gtk_widget_set_usize (mainWindow, WIN_WIDTH, WIN_HEIGHT);
	gtk_window_set_position(GTK_WINDOW(mainWindow), WIN_POSITION);
	if ((tfmIcon = create_pixbuf())!=NULL) gtk_window_set_icon(GTK_WINDOW(mainWindow), tfmIcon);
	gtk_widget_realize(mainWindow);
	fullscreen = FALSE;

	if (flashPlayerArg!=NULL)
	{
		FILE *f = fopen(flashPlayerArg, "r");
		if (f==NULL) flashLocation = find_file(flashPlayerNames, flashPlayerPaths, sizeof(flashPlayerNames)/sizeof(char*), sizeof(flashPlayerPaths)/sizeof(char*));
		else
		{
			flashLocation = strdup(flashPlayerArg);
			fclose(f);
		}
	}
	else flashLocation = find_file(flashPlayerNames, flashPlayerPaths, sizeof(flashPlayerNames)/sizeof(char*), sizeof(flashPlayerPaths)/sizeof(char*));

	if (flashLocation==NULL)
	{
		char *text = "Flash Player not found on your machine.\n" \
			"Try install it, using package manager of your distribution, or download it from https://get.adobe.com/ru/flashplayer/, and install with instruction.\n" \
			"If problem appears again, try specify --flash (-f) argument followed by path to your Flash Player shared object or recompile with FLASH_SO_LOCATION/FLASH_SO_NAME string definitions.\nFlash Player searches in following places:\n",
			*msg = malloc(strlen(text)+1), *ts;
		int i, len = strlen(text);
		msg[0] = 0;
		sprintf(msg, "%s", text);
		for (i = 0; i < sizeof(flashPlayerPaths)/sizeof(char*); i++)
		{
			msg = realloc(msg, (len+=strlen(flashPlayerPaths[i])+3));
			ts = strdup(msg);
			sprintf(msg, "%s\t%s\n", ts, flashPlayerPaths[i]);
			free(ts);
		}
		text = "And by following file names:\n";
		msg = realloc(msg, (len+=strlen(text)+1));
		strcat(msg, text);
		for (i = 0; i < sizeof(flashPlayerNames)/sizeof(char*); i++)
		{
			msg = realloc(msg, (len+=strlen(flashPlayerNames[i])+3));
			ts = strdup(msg);
			sprintf(msg, "%s\t%s\n", ts, flashPlayerNames[i]);
			free(ts);
		}
		msg[len-1] = 0;
		perr(msg);
	}
	else if (!(flashPlugin = dlopen(flashLocation, RTLD_LAZY | RTLD_LOCAL))) perr("cannot load file %s: %s", flashLocation, dlerror()); // Loading plugin
	else free(flashLocation);

	// Load functions
	NP_Initialize = (NPError(*)(NPNetscapeFuncs* bFuncs, NPPluginFuncs* pFuncs))loadSymbol(flashPlugin, "NP_Initialize");
	NP_Shutdown = (NPError(*)())loadSymbol(flashPlugin, "NP_Shutdown");
	
	initBrowserFuncs(&bFuncs); // Initialize browser functions

	NP_Initialize(&bFuncs, &pFuncs); // Loading entry points

	// Initialize GTK+ widget
	gtk_widget_set_parent_window(socketWidget, mainWindow->window);

	g_signal_connect(socketWidget, "plug_removed", G_CALLBACK(plug_removed_cb), NULL);
	g_signal_connect(socketWidget, "destroy", G_CALLBACK(gtk_widget_destroyed), &socketWidget);

	gdk_window_get_user_data(mainWindow->window, &user_data);

	container = GTK_CONTAINER(user_data);
	gtk_container_add(container, socketWidget);
	gtk_widget_realize(socketWidget);

	new_allocation.x = 0;
	new_allocation.y = 0;
	new_allocation.width = WIN_WIDTH;
	new_allocation.height = WIN_HEIGHT;
	gtk_widget_size_allocate(socketWidget, &new_allocation);

	gtk_widget_show(socketWidget);
	gdk_flush();

	ww = gtk_socket_get_id(GTK_SOCKET(socketWidget));
	w = gdk_window_lookup(ww);

	npwin = (NPWindow*)malloc(sizeof(NPWindow));
	npwin->window = (void*)(unsigned long)ww;
	npwin->x = 0;
	npwin->y = 0;
	npwin->width = WIN_WIDTH;
	npwin->height = WIN_HEIGHT;

	ws_info = (NPSetWindowCallbackStruct*)malloc(sizeof(NPSetWindowCallbackStruct));
	ws_info->type = NP_SETWINDOW;
	ws_info->display = GDK_WINDOW_XDISPLAY(w);
	ws_info->colormap = GDK_COLORMAP_XCOLORMAP(gdk_drawable_get_colormap(w));
	GdkVisual* gdkVisual = gdk_drawable_get_visual(w);
	ws_info->visual = GDK_VISUAL_XVISUAL(gdkVisual);
	ws_info->depth = gdkVisual->depth;

	npwin->ws_info = ws_info;
	npwin->type = NPWindowTypeWindow;

	// Complete initialization
	instancep = (NPP_t*)malloc(sizeof(NPP_t));
	memset(instancep, 0, sizeof(sizeof(NPP_t)));
	instance = instancep;

	NPSavedData* saved = (NPSavedData*)malloc(sizeof(NPSavedData));
	memset(saved, 0, sizeof(sizeof(NPSavedData)));

	gtk_widget_show_all(mainWindow);

	// Loading TransformiceChargeur.swf
	stream = (NPStream*)malloc(sizeof(NPStream));

	stream->url = strdup("http://www.transformice.com/TransformiceChargeur.swf");
	stream->ndata = 0;
	stream->end = 99782;
	stream->lastmodified = 1201822722;
	stream->notifyData = 0x00000000;

	pFuncs.newp("application/x-shockwave-flash", instance, NP_EMBED, 8, xargv, xargm, 0);
	pFuncs.getvalue(instance, NPPVpluginScriptableNPObject, &object);
	pFuncs.setwindow(instance, npwin);
	pFuncs.newstream(instance, "application/x-shockwave-flash", stream, 0, &stype);

	pFuncs.writeready(instance, stream);
	pFuncs.write(instance, stream, 0, loader_data_end-loader_data, loader_data);

	pFuncs.destroystream(instance, stream, NPRES_DONE);
	free((void*)stream->url);
	free(stream);

	// Event signals
	g_signal_connect(G_OBJECT(mainWindow), "destroy", G_CALLBACK (destroy), NULL);
	g_signal_connect(G_OBJECT(mainWindow), "configure-event", G_CALLBACK (winChanged), NULL);
	g_signal_connect(G_OBJECT (mainWindow), "key-press-event", G_CALLBACK (keyPressed), NULL);
	g_signal_connect(G_OBJECT (mainWindow), "key-release-event", G_CALLBACK (keyReleased), NULL);

	gtk_main();

	pFuncs.destroy(instance, &savedData);
	NP_Shutdown();
	dlclose(flashPlugin);
	return 0;
}
