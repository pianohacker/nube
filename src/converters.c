#include <glib-object.h>

#include "converters.h"

GData *known_converters;

void nube_converter_register(const gchar *name, NubeConverterFunc convert_func) {
	g_datalist_set_data(&known_converters, name, convert_func);
}

void nube_builtin_converters_init() {
}

NubeConverterFunc* nube_converter_get(const GQuark id) {
	return g_datalist_id_get_data(&known_converters, id);
}
