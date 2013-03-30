#include <glib.h>

#include "sources.h"
#include "source_providers.h"

static GData *source_providers;

void nube_source_provider_register(const gchar *name, NubeSourceProvideFunc provide_func) {
	g_datalist_set_data(source_providers, name, provide_func);
}

void nube_source_provide(const gchar *name, NubeSource *source, gchar* const *attr_names, GValue* const *attr_values) {
	NubeSourceProvideFunc provide_func = g_datalist_get_data(&source_providers, name);

	if (!provide_func) {
		g_printerr("Unknown source provider: %s\n", name);
		exit(1);
	}

	g_datalist_init(&source->data);
	provide_func(source, attr_names, attr_values);
}
