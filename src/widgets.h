#ifndef __WIDGETS_H__
#define __WIDGETS_H__

#include "config.h"

void nube_widget_add(ClutterActor *panel, const NubeWidgetConfig *widget_config);

void nube_widget_type_register(const gchar *name, ClutterActor* (*init_func)(NubeWidgetConfig *widget_config), void (*draw_func)(ClutterActor *actor, NubeWidgetConfig *widget_config));
void nube_builtin_widget_types_init();

void nube_update_all(ClutterActor *container);


#endif
