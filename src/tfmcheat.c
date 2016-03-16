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

#define URL	"http://www.transformice.com/TransformiceChargeur.swf"

#define WIN_WIDTH		800
#define WIN_HEIGHT		600

#define WIN_POSITION	GTK_WIN_POS_CENTER

#define FLASH_PLUGIN_SO	"/usr/lib/nsbrowser/plugins/libflashplayer.so"

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
	""
};

GdkPixbuf *create_pixbuf(const gchar *filename)
{
	int i;
	GdkPixbuf *pixbuf;
	GError *error = NULL;
	pixbuf = gdk_pixbuf_new_from_file(filename, &error);
	for (i = 0; i < sizeof(iconPaths)/sizeof(char*); i++)
	{
		char *path = (char*)malloc(strlen(filename)+strlen(iconPaths[i])+2);
		sprintf(path, "%s/%s", iconPaths[i], filename);
		pixbuf = gdk_pixbuf_new_from_file(path, NULL);
		if (pixbuf) break;
	}
	return pixbuf;
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

int main(int argc, char *argv[])
{
	void *flashPlugin;
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

	char *xargv[]= {"allowscriptaccess", "name", "quality", "wmode", "allowFullScreen", "width", "height", "scale"},
		*xargm[]= {"always", "Transformice", "best", "direct", "true", "800", "600","exactfit"};

	pname = argv[0];

	gtk_init (&argc, &argv);
	// GTK+ window
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainWindow), "Transformice");
	gtk_widget_set_usize (mainWindow, WIN_WIDTH, WIN_HEIGHT);
	gtk_window_set_position(GTK_WINDOW(mainWindow), WIN_POSITION);
	gtk_window_set_icon(GTK_WINDOW(mainWindow), create_pixbuf("/usr/share/pixmaps/transformice.png"));
	gtk_widget_realize(mainWindow);
	fullscreen = FALSE;

	if (!(flashPlugin = dlopen(FLASH_PLUGIN_SO, RTLD_LAZY | RTLD_LOCAL))) perr("cannot load %s: %s", FLASH_PLUGIN_SO, dlerror()); // Loading plugin

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

	stream->url = strdup(URL);
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
