#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib-object.h>
#include <stdarg.h>
#include <stdbool.h>

typedef struct {
	double x;
	double y;
} DPoint;

char* nube_get_runtime_path(char *argv0);

double nube_get_num_file(const char *path);
char* nube_get_str_file(const char *path);

void nube_datalist_require_value(const gchar *container_name, GData *datalist, const gchar *key, GType type, ...);
bool nube_datalist_id_get_value(GData *datalist, GQuark key_id, GType type, ...);
#define nube_datalist_get_value(datalist, key, type, ...) nube_datalist_id_get_value(datalist, g_quark_from_string(key), type, __VA_ARGS__)
bool nube_datalist_id_get_value_v(GData *datalist, GQuark key_id, GType type, va_list args);

#endif
