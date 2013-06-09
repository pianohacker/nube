#include <glib.h>

#include "sources.h"
#include "source_providers.h"

static GData *source_providers;

void nube_source_provider_register(const gchar *name, NubeSourceProvideFunc provide_func) {
	g_datalist_set_data(&source_providers, name, provide_func);
}

#include "builtin_source_providers.c"

void nube_source_provide(const gchar *name, const gchar *provider_name, const gchar *converter_name, GData *attributes) {
	NubeSourceProvideFunc *provide_func = g_datalist_get_data(&source_providers, provider_name);

	if (!provide_func) {
		g_printerr("Unknown source provider: %s\n", provider_name);
		exit(1);
	}

	provide_func(name, attributes);

	if (converter_name) {
		nube_source_set_converter(g_quark_from_string(name), converter_name);
	}
}
