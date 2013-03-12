#ifndef __DRAWING_H__
#define __DRAWING_H__

#include "config.h"

extern void nube_canvas_resize_cb(ClutterActor *actor, GParamSpec *spec, ClutterCanvas *canvas);
extern void nube_draw_panel_poly(ClutterActor *actor, cairo_t *cr, gfloat width, gfloat height, NubePanelConfig *panel_config);

#endif
