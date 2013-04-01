#include <glib.h>

#include "sources.h"
#include "source_providers.h"

static GData *source_providers;

void nube_source_provider_register(const gchar *name, NubeSourceProvideFunc provide_func) {
	g_datalist_set_data(&source_providers, name, provide_func);
}

#include "builtin_source_providers.c"

NubeSource nube_source_provide(const gchar *name, GData *attributes) {
	NubeSourceProvideFunc provide_func = g_datalist_get_data(&source_providers, name);

	if (!provide_func) {
		g_printerr("Unknown source provider: %s\n", name);
		exit(1);
	}

	return provide_func(source, attributes);
}
