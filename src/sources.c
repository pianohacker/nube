#include <glib.h>
#include <gobject/gvaluecollector.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

GData *available_sources;
GData *used_sources;

typedef struct __NubeSource {
	void (*init_func)(struct __NubeSource *source);
	void (*update_func)(struct __NubeSource *source);
	GData *last_data;
	GData *internal_data;
} _NubeSource;

void _init_source(gpointer key, GQuark source_id, gpointer user_data) {
	_NubeSource *source = g_datalist_id_get_data(&available_sources, source_id);

	if (!source) {
		g_printerr("Unknown source: %s\n", g_quark_to_string(source_id));
		exit(1);
	}

	g_datalist_init(&source->last_data);
	g_datalist_init(&source->internal_data);
	source->init_func(source);

	g_datalist_id_set_data(&used_sources, source_id, source);
}

void nube_sources_start(GHashTable *referenced_sources) {
	g_hashtable_foreach(referenced_sources, (GHFunc) _init_source, NULL);
}

void _update_source(GQuark source_id, _NubeSource *source, gpointer user_data) {
	source->update_func(source);
}

void nube_sources_update() {
	g_datalist_foreach(&used_sources, (GDataForeachFunc) _update_source, NULL);
}

void nube_source_get_id(GQuark source_id, GQuark item_id, ...) {
	va_list args;
	va_start(args);

	_NubeSource *source = g_datalist_id_get_data(&used_source, source_id);
	g_return_if_fail(source != NULL);

	GValue *value = g_datalist_id_get_data(&source->last_data, item_id);
	g_return_if_fail(value != NULL);

	gchar *err;
	G_VALUE_LCOPY(value, args, 0, err);
	g_return_if_fail(err == NULL);

	va_end(args);
}
