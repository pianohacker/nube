#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <glib-object.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include "config.h"
#include "drawing.h"
#include "panel.h"
#include "util.h"
#include "widgets.h"

struct {
	gfloat width;
	gfloat height;

	ClutterActor *stage;
	ClutterTransition *show_trans;
	ClutterTransition *hide_trans;

	ClutterActor *top_panel;
	ClutterActor *right_panel;
	ClutterActor *bottom_panel;
	ClutterActor *left_panel;
} nube;

char *runtime_path;

gboolean nube_update_widgets(gpointer user_data) {
	nube_update_all(nube.top_panel);
	nube_update_all(nube.right_panel);
	nube_update_all(nube.bottom_panel);
	nube_update_all(nube.left_panel);

	return TRUE;
}

void setup_stage(ClutterStage *stage, gpointer *data) {
	clutter_actor_get_size(CLUTTER_ACTOR(stage), &nube.width, &nube.height);

	nube.top_panel = nube_panel_new(
		-GLOW_SIZE, -GLOW_SIZE,
		-GLOW_SIZE, -GLOW_SIZE * 2 - TOP_PANEL_OPTIONS.size,
		nube.width, TOP_PANEL_OPTIONS.size,

		&TOP_PANEL_OPTIONS
	);
	clutter_actor_add_child(nube.stage, nube.top_panel);

	nube.right_panel = nube_panel_new(
		nube.width - RIGHT_PANEL_OPTIONS.size - GLOW_SIZE, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		nube.width, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		RIGHT_PANEL_OPTIONS.size, nube.height - TOP_PANEL_OPTIONS.size - BOTTOM_PANEL_OPTIONS.size - GUTTER_SIZE * 2,

		&RIGHT_PANEL_OPTIONS
	);
	clutter_actor_add_child(nube.stage, nube.right_panel);

	nube.bottom_panel = nube_panel_new(
		-GLOW_SIZE, nube.height - BOTTOM_PANEL_OPTIONS.size - GLOW_SIZE,
		-GLOW_SIZE, nube.height,
		nube.width, BOTTOM_PANEL_OPTIONS.size,

		&BOTTOM_PANEL_OPTIONS
	);
	clutter_actor_add_child(nube.stage, nube.bottom_panel);

	nube.left_panel = nube_panel_new(
		-GLOW_SIZE, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		-GLOW_SIZE * 2 - LEFT_PANEL_OPTIONS.size, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		LEFT_PANEL_OPTIONS.size, nube.height -TOP_PANEL_OPTIONS.size - BOTTOM_PANEL_OPTIONS.size - GUTTER_SIZE * 2,

		&LEFT_PANEL_OPTIONS
	);
	clutter_actor_add_child(nube.stage, nube.left_panel);

	clutter_actor_hide(nube.stage);
}

void add_opacity_transition(ClutterTransition *group, ClutterActor *actor, guint duration, guint to) {
	ClutterTransition *result = clutter_property_transition_new("opacity");
	guint from = clutter_actor_get_opacity(actor);

	clutter_transition_set_animatable(result, CLUTTER_ANIMATABLE(actor));
	clutter_transition_set_interval(result, clutter_interval_new(G_TYPE_UINT, from, to));

	clutter_timeline_set_duration(CLUTTER_TIMELINE(result), duration);

	clutter_transition_group_add_transition(CLUTTER_TRANSITION_GROUP(group), result);
}

void add_slide_transition(ClutterTransition *group, ClutterActor *actor, guint duration, const char *to) {
	ClutterTransition *result_x = clutter_property_transition_new("x");
	ClutterTransition *result_y = clutter_property_transition_new("y");
	gfloat x, y;
	clutter_actor_get_position(actor, &x, &y);
	ClutterPoint *to_point = g_object_get_data(G_OBJECT(actor), to);

	clutter_transition_set_animatable(result_x, CLUTTER_ANIMATABLE(actor));
	clutter_transition_set_animatable(result_y, CLUTTER_ANIMATABLE(actor));
	clutter_transition_set_interval(result_x, clutter_interval_new(G_TYPE_FLOAT, x, to_point->x));
	clutter_transition_set_interval(result_y, clutter_interval_new(G_TYPE_FLOAT, y, to_point->y));

	clutter_timeline_set_duration(CLUTTER_TIMELINE(result_x), duration);
	clutter_timeline_set_duration(CLUTTER_TIMELINE(result_y), duration);
	clutter_timeline_set_progress_mode(CLUTTER_TIMELINE(result_x), CLUTTER_EASE_OUT_CIRC);
	clutter_timeline_set_progress_mode(CLUTTER_TIMELINE(result_y), CLUTTER_EASE_OUT_CIRC);

	clutter_transition_group_add_transition(CLUTTER_TRANSITION_GROUP(group), result_x);
	clutter_transition_group_add_transition(CLUTTER_TRANSITION_GROUP(group), result_y);
}

void show_stage() {
	clutter_actor_show(nube.stage);

	clutter_timeline_stop(CLUTTER_TIMELINE(nube.hide_trans));
	clutter_timeline_stop(CLUTTER_TIMELINE(nube.show_trans));
	clutter_transition_group_remove_all(CLUTTER_TRANSITION_GROUP(nube.show_trans));

	add_opacity_transition(nube.show_trans, nube.stage, SHOW_LEN, 255);
	add_slide_transition(nube.show_trans, nube.top_panel, SHOW_LEN, "shown_point");
	add_slide_transition(nube.show_trans, nube.right_panel, SHOW_LEN, "shown_point");
	add_slide_transition(nube.show_trans, nube.bottom_panel, SHOW_LEN, "shown_point");
	add_slide_transition(nube.show_trans, nube.left_panel, SHOW_LEN, "shown_point");

	clutter_timeline_start(CLUTTER_TIMELINE(nube.show_trans));
}

void hide_done() {
	clutter_actor_hide(nube.stage);
}

void hide_stage() {
	clutter_timeline_stop(CLUTTER_TIMELINE(nube.show_trans));
	clutter_timeline_stop(CLUTTER_TIMELINE(nube.hide_trans));
	clutter_transition_group_remove_all(CLUTTER_TRANSITION_GROUP(nube.hide_trans));

	add_opacity_transition(nube.hide_trans, nube.stage, HIDE_LEN, 0);
	add_slide_transition(nube.hide_trans, nube.top_panel, HIDE_LEN, "hidden_point");
	add_slide_transition(nube.hide_trans, nube.right_panel, HIDE_LEN, "hidden_point");
	add_slide_transition(nube.hide_trans, nube.bottom_panel, HIDE_LEN, "hidden_point");
	add_slide_transition(nube.hide_trans, nube.left_panel, HIDE_LEN, "hidden_point");

	clutter_timeline_start(CLUTTER_TIMELINE(nube.hide_trans));
}

ClutterX11FilterReturn filter_func(XEvent *xev, ClutterEvent *cev, Window *root_window) {
	if (xev->xany.window == *root_window) {
		switch (xev->xany.type) {
			case KeyPress:
				show_stage();
				break;
			case KeyRelease:
				hide_stage();
				break;
		}
	}

	return CLUTTER_X11_FILTER_CONTINUE;
}

int main(int argc, char **argv) {
	runtime_path = strcat(strcat(malloc(PATH_MAX), nube_get_runtime_path(argv[0])), "/share");

	clutter_x11_set_use_argb_visual(TRUE);
	if (clutter_init(&argc, &argv) != CLUTTER_INIT_SUCCESS) {
	   return 1;
	}

	nube.show_trans = clutter_transition_group_new();
	clutter_timeline_set_duration(CLUTTER_TIMELINE(nube.show_trans), SHOW_LEN);
	nube.hide_trans = clutter_transition_group_new();
	clutter_timeline_set_duration(CLUTTER_TIMELINE(nube.hide_trans), HIDE_LEN);
	g_signal_connect(nube.hide_trans, "completed", G_CALLBACK(hide_done), NULL);

	ClutterColor stage_color = BACK_COLOR;

	nube.stage = clutter_stage_new();
	clutter_stage_set_use_alpha(CLUTTER_STAGE(nube.stage), TRUE);
	clutter_actor_set_background_color(nube.stage, &stage_color);
	g_signal_connect(nube.stage, "destroy", clutter_main_quit, NULL);
	g_signal_connect(nube.stage, "fullscreen", G_CALLBACK(setup_stage), NULL);

	clutter_stage_set_fullscreen(CLUTTER_STAGE(nube.stage), TRUE);
	clutter_actor_set_opacity(nube.stage, 0);
	clutter_actor_show(nube.stage);

	Display *display = clutter_x11_get_default_display();
	Window root_window = DefaultRootWindow(display); 

	XGrabKey(
		display,
		XKeysymToKeycode(display, XK_Super_L),
		AnyModifier,
		root_window,
		false,
		GrabModeAsync,
		GrabModeAsync
	);
	clutter_x11_add_filter((ClutterX11FilterFunc) filter_func, &root_window);

	clutter_threads_add_timeout(UPDATE_DELAY, nube_update_widgets, NULL);

	clutter_main();

	return 0;
}
