#include <glib.h>
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
	g_datalist_clear(&source->last_data);
	source->update_func(source);
}

void nube_sources_update() {
	g_datalist_foreach(&used_sources, (GDataForeachFunc) _update_source, NULL);
}

bool nube_source_get(GQuark source_id, GQuark item, void *output) {
}
