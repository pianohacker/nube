#ifndef __SOURCE_PROVIDERS_H__
#define __SOURCE_PROVIDERS_H__

#include <glib.h>
#include <stdbool.h>

#include "sources.h"

typedef NubeSource* (NubeSourceProvideFunc)(NubeSource *source, gchar* const *attr_names, GValue* const *attr_values);

void nube_source_provider_register(const gchar *name, NubeSourceProvideFunc provide_func);
void nube_builtin_source_providers_init();

void nube_source_provide(const gchar *name, NubeSource *source, gchar* const *attr_names, GValue* const *attr_values);

#endif
