#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <glib.h>
#include <glib-object.h>
#include <stdbool.h>

typedef struct _NubeSource {
	void (*init_func)(struct _NubeSource *source, gpointer user_data);
	void (*update_func)(struct _NubeSource *source, gpointer user_data);
	GQuark converter_id;
	GData *data;
	gpointer user_data;

	glong intervals_delay;
	glong intervals_left;
} NubeSource;

void nube_source_register(const gchar *name, void (*init_func)(struct _NubeSource *source, gpointer user_data), void (*update_func)(struct _NubeSource *source, gpointer user_data), gpointer user_data);
void nube_builtin_sources_init();

void nube_sources_start(GHashTable *referenced_sources);
void nube_sources_update();

void nube_source_set_converter(GQuark source_id, const char *converter);
void nube_source_set_update_delay(GQuark source_id, glong update_delay);

bool nube_source_get_id(GQuark source_id, GQuark item_id, GType type, ...);
#define nube_source_get(source_id, item, type, ...) nube_source_get_id(source_id, g_quark_from_string(item), type, __VA_ARGS__)

#endif
