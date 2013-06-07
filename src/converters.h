#ifndef __CONVERTERS_H__
#define __CONVERTERS_H__

#include <glib-object.h>

typedef GValue* (NubeConverterFunc)(const GQuark id, GValue *value);

void nube_converter_register(const gchar *name, NubeConverterFunc convert_func);
void nube_builtin_converters_init();

NubeConverterFunc* nube_converter_get(const GQuark id);

#endif
