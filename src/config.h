#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <clutter/clutter.h>

typedef struct {
	ClutterColor fill_color;
	ClutterColor stroke_color;

	gfloat top_right;
	gfloat bottom_right;
	gfloat bottom_left;
	gfloat top_left;

	gfloat size;
	gfloat border_width;
} NubeCutRectOptions;

#define BACK_COLOR { 208, 48, 3, 50 }
#define BG_COLOR { 32, 34, 35, 208 }
#define BORDER_COLOR { 249, 249, 249, 255 }
#define GUTTER_SIZE 40
#define GLOW_SIZE 20

ClutterColor BG_COLOR_VAL = BG_COLOR;

NubeCutRectOptions TOP_PANEL_OPTIONS = {
	BG_COLOR,
	BORDER_COLOR,
	
	0, 100, 100, 0,
	
	80,
	0
};

NubeCutRectOptions RIGHT_PANEL_OPTIONS = {
	BG_COLOR, BORDER_COLOR,
	
	0, 0, 60, 40,

	160,
	0
};

NubeCutRectOptions BOTTOM_PANEL_OPTIONS = {
	BG_COLOR, BORDER_COLOR,
	
	40, 0, 0, 40,
	
	40,
	0
};

NubeCutRectOptions LEFT_PANEL_OPTIONS = {
	BG_COLOR, BORDER_COLOR,
	
	40, 60, 0, 0,
	
	160,
	0
};

#endif
