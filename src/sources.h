#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <glib.h>
#include <stdbool.h>

void nube_sources_init();
void nube_sources_start(GHashTable *referenced_sources);
void nube_sources_update();
bool nube_source_get_id(GQuark source_id, GQuark item_id, ...);
#define nube_source_get(source_id, item, ...) nube_source_get_id(source_id, g_quark_from_string(item), __VA_ARGS__)

#endif
