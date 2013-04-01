#ifndef __SOURCE_PROVIDERS_H__
#define __SOURCE_PROVIDERS_H__

#include <glib.h>
#include <stdbool.h>

#include "sources.h"

typedef NubeSource (NubeSourceProvideFunc)(GData *attributes);

void nube_source_provider_register(const gchar *name, NubeSourceProvideFunc provide_func);
void nube_builtin_source_providers_init();

NubeSource nube_source_provide(const gchar *name, GData *attributes);

#endif
