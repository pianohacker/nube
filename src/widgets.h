#ifndef __WIDGETS_H__
#define __WIDGETS_H__

#include "config.h"

extern void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeWidgetConfig *widget_config);
void nube_widget_update(ClutterActor *widget, NubeWidgetConfig *widget_config);
void nube_widget_add(ClutterActor *panel, const NubeWidgetConfig *widget_config);

void nube_widget_type_register(const gchar *name, ClutterActor* (*init_func)(const NubeWidgetConfig *widget_config), void (*draw_func)(ClutterActor *actor, const NubeWidgetConfig *widget_config));
void nube_widget_types_init();

void nube_update_all(ClutterActor *container);


#endif
