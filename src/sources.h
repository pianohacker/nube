#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <glib.h>
#include <stdbool.h>

typedef struct _NubeSource {
	void (*init_func)(struct _NubeSource *source);
	void (*update_func)(struct _NubeSource *source);
	GData *data;
} NubeSource;

void nube_source_register(const gchar *name, void (*init_func)(struct _NubeSource *source), void (*update_func)(struct _NubeSource *source));
void nube_builtin_sources_init();

void nube_sources_start(GHashTable *referenced_sources);
void nube_sources_update();

bool nube_source_get_id(GQuark source_id, GQuark item_id, ...);
#define nube_source_get(source_id, item, ...) nube_source_get_id(source_id, g_quark_from_string(item), __VA_ARGS__)

#endif
