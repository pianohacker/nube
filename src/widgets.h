#ifndef __WIDGETS_H__
#define __WIDGETS_H__

extern void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeWidgetOptions *opts);
extern void nube_update_widget(ClutterActor *widget, NubeWidgetOptions *opts);
extern void nube_add_widget(ClutterLayoutManager *layout, const NubeWidgetOptions *opts);
extern void nube_update_all(ClutterActor *container);

#endif
