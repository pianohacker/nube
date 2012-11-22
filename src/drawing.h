#ifndef __DRAWING_H__
#define __DRAWING_H__

extern void nube_canvas_resize_cb(ClutterActor *actor, GParamSpec *spec, ClutterCanvas *canvas);
extern void nube_draw_cut_rect(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubePanelOptions *opts);

#endif
