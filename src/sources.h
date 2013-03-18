#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <glib.h>
#include <stdbool.h>

void nube_sources_init();
void nube_sources_start(GHashTable *referenced_sources);
void nube_sources_update();
bool nube_source_get(GQuark source_id, GQuark item, void *output);

#endif
