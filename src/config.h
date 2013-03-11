#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <clutter/clutter.h>
#include <stdint.h>

//> Typedefs

typedef struct {
	GQuark type_id;
	GQuark source_id;

	ClutterPoint *position;
	ClutterPoint *pivot_point;

	GDataList *props;
} NubeWidgetConfig;

typedef struct {
	gdouble position;

	gdouble shape_top;
	gdouble shape_left;
	gdouble shape_right;
	gdouble shape_bottom;
	GArray *shape_elems;

	ClutterColor *background;

	GArray *widgets;
} NubePanelConfig;

typedef struct {
	ClutterColor *background;
	ClutterColor *fg;
	gdouble glow_size;
	gint show_time;
	gint hide_time;
	gint update_delay;

	NubePanelConfig top_panel;
	NubePanelConfig left_panel;
	NubePanelConfig right_panel;
	NubePanelConfig bottom_panel;
} NubeGlobalConfig;

extern NubeGlobalConfig nube_config;

//> Appearance Options

#define WIDGET_PARTIAL_COLOR { 255, 255, 255, 80 }

//> Paths
#define BATTERY_PREFIX "/sys/class/power_supply/BAT0"

#endif
