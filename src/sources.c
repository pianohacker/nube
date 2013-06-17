#include <glib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "converters.h"
#include "sources.h"
#include "util.h"

GData *available_sources;
GData *used_sources;

void _init_source(gpointer key, GQuark source_id, gpointer user_data) {
	NubeSource *source = g_datalist_id_get_data(&available_sources, source_id);

	if (!source) {
		g_printerr("Unknown source: %s\n", g_quark_to_string(source_id));
		exit(1);
	}

	g_datalist_init(&source->data);
	source->init_func(source, source->user_data);

	g_datalist_id_set_data(&used_sources, source_id, source);
}

void nube_source_register(const gchar *name, void (*init_func)(NubeSource *source, gpointer user_data), void (*update_func)(NubeSource *source, gpointer user_data), gpointer user_data) {
	NubeSource *source = g_slice_new0(NubeSource);

	source->init_func = init_func;
	source->update_func = update_func;
	source->user_data = user_data;
	source->intervals_delay = 1;
	source->intervals_left = 1;

	g_datalist_id_set_data(&available_sources, g_quark_from_static_string(name), source);
}

#include "builtin_sources.c"

void nube_sources_start(GHashTable *referenced_sources) {
	g_hash_table_foreach(referenced_sources, (GHFunc) _init_source, NULL);
}

void _update_source(GQuark source_id, NubeSource *source, gpointer user_data) {
	source->intervals_left--;

	if (source->intervals_left == 0) {
		source->update_func(source, source->user_data);

		GValue *value;
		if (source->converter_id && (value = g_datalist_get_data(&source->data, "value"))) {
			(*nube_converter_get(source->converter_id))(g_quark_from_string("value"), value);
		}

		source->intervals_left = source->intervals_delay;
	}
}

void nube_sources_update() {
	g_datalist_foreach(&used_sources, (GDataForeachFunc) _update_source, NULL);
}

void nube_source_set_update_delay(GQuark source_id, glong update_delay) {
	NubeSource *source = g_datalist_id_get_data(&available_sources, source_id);
	g_return_if_fail(source != NULL);

	// Intentional integer division, so if the update delay is a non-integer
	// multiple of the global delay, the source will err on the side of more
	// frequent updates.
	source->intervals_delay = update_delay / nube_config.update_delay;
}

void nube_source_set_converter(GQuark source_id, const char *converter) {
	NubeSource *source = g_datalist_id_get_data(&available_sources, source_id);
	g_return_if_fail(source != NULL);

	source->converter_id = g_quark_from_string(converter);

	if (!nube_converter_get(source->converter_id)) {
		g_printerr("Unknown converter: %s\n", converter);
		exit(1);
	}
}

bool nube_source_get_id(GQuark source_id, GQuark item_id, GType type, ...) {
	va_list args;
	va_start(args, type);

	NubeSource *source = g_datalist_id_get_data(&used_sources, source_id);
	// As all known sources should have been setup in _sources_start,
	// this is a programming error in the widget
	g_return_val_if_fail(source != NULL, false);

	bool result = nube_datalist_id_get_value_v(source->data, item_id, type, args);

	va_end(args);

	return result;
}
