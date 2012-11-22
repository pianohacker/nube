#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <clutter/clutter.h>

//> Typedefs

#define MAX_WIDGETS_SIZE 16

typedef struct {
	enum {
	   WIDGET_END = 0,
	   WIDGET_CLOCK,
	   WIDGET_BATTERY
	} type;
	gint row;
	gint column;
	gint width;
	gint height;
	ClutterActorAlign x_align;
	ClutterActorAlign y_align;
	char *config;
} NubeWidgetOptions;

typedef struct {
	ClutterColor fill_color;
	ClutterColor stroke_color;

	gfloat top_right;
	gfloat bottom_right;
	gfloat bottom_left;
	gfloat top_left;

	gfloat top_padding;
	gfloat right_padding;
	gfloat bottom_padding;
	gfloat left_padding;

	gfloat size;
	gfloat border_width;

	NubeWidgetOptions widget_opts[MAX_WIDGETS_SIZE];
} NubePanelOptions;

//> Simple Options

#define BACK_COLOR { 208, 48, 3, 50 }
#define BG_COLOR { 32, 34, 35, 128 }
#define FG_COLOR { 255, 255, 255, 240 }
#define BORDER_COLOR { 249, 249, 249, 255 }

#define WIDGET_PARTIAL_COLOR { 255, 255, 255, 80 }

#define GUTTER_SIZE 40
#define GLOW_SIZE 25

#define SHOW_LEN 240
#define HIDE_LEN 384

#define DEFAULT_FONT "Exo 12"

static const NubePanelOptions TOP_PANEL_OPTIONS = {
	BG_COLOR,
	BORDER_COLOR,
	
	0, 100, 100, 0,
	10, 100, 10, 100,
	
	80,
	0,

	{
		{ 
			WIDGET_CLOCK,
			0, 0,
			1, 1,
			CLUTTER_ACTOR_ALIGN_START, CLUTTER_ACTOR_ALIGN_FILL,
			"%a, %b %e\nExo 24"
		},
		{ 
			WIDGET_CLOCK,
			0, 1,
			1, 1,
			CLUTTER_ACTOR_ALIGN_END, CLUTTER_ACTOR_ALIGN_FILL,
			"%I:%M %p\nExo 24"
		},
	}
};

static const NubePanelOptions RIGHT_PANEL_OPTIONS = {
	BG_COLOR, BORDER_COLOR,
	
	0, 0, 80, 40,
	80, 10, 40, 10,

	160,
	0,

	{
	}
};

static const NubePanelOptions BOTTOM_PANEL_OPTIONS = {
	BG_COLOR, BORDER_COLOR,
	
	40, 0, 0, 40,
	10, 100, 10, 100,
	
	40,
	0,

	{
	}
};

static const NubePanelOptions LEFT_PANEL_OPTIONS = {
	BG_COLOR, BORDER_COLOR,
	
	40, 80, 0, 0,
	40, 10, 80, 10,
	
	160,
	0,

	{
		{ 
			WIDGET_BATTERY,
			0, 0,
			1, 1,
			CLUTTER_ACTOR_ALIGN_END, CLUTTER_ACTOR_ALIGN_FILL,
			"10\n10"
		},
	}
};

//> Paths
#define BATTERY_PREFIX "/sys/class/power_supply/BAT0"

#endif
