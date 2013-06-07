#ifndef __SOURCE_PROVIDERS_H__
#define __SOURCE_PROVIDERS_H__

#include <glib.h>
#include <stdbool.h>

#include "sources.h"

typedef void (NubeSourceProvideFunc)(const gchar *name, GData *attributes);

void nube_source_provider_register(const gchar *name, NubeSourceProvideFunc provide_func);
void nube_builtin_source_providers_init();

void nube_source_provide(const gchar *name, const gchar *provider_name, const gchar *converter_name, GData *attributes);

#endif
