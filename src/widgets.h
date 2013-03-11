#ifndef __WIDGETS_H__
#define __WIDGETS_H__

#include "config.h"

extern void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeWidgetConfig *widget_config);
extern void nube_update_widget(ClutterActor *widget, NubeWidgetConfig *widget_config);
extern void nube_add_widget(ClutterLayoutManager *layout, const NubeWidgetConfig *widget_config);
extern void nube_update_all(ClutterActor *container);

#endif
