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
#include "sources.h"
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
	nube_sources_update();
	nube_update_all(nube.top_panel);
	nube_update_all(nube.right_panel);
	nube_update_all(nube.bottom_panel);
	nube_update_all(nube.left_panel);

	return TRUE;
}

void setup_stage(ClutterStage *stage, gpointer *data) {
	clutter_actor_get_size(CLUTTER_ACTOR(stage), &nube.width, &nube.height);

	nube.top_panel = nube_panel_new(
		nube_config.top_panel.position, 0,
		0, -1,

		&nube_config.top_panel
	);
	clutter_actor_add_child(nube.stage, nube.top_panel);

	nube.right_panel = nube_panel_new(
		nube.width - nube_config.right_panel.shape_max_x, nube_config.right_panel.position,
		1, 0,

		&nube_config.right_panel
	);
	clutter_actor_add_child(nube.stage, nube.right_panel);

	nube.bottom_panel = nube_panel_new(
		nube_config.bottom_panel.position, nube.height - nube_config.bottom_panel.shape_max_y,
		0, 1,

		&nube_config.bottom_panel
	);
	clutter_actor_add_child(nube.stage, nube.bottom_panel);

	nube.left_panel = nube_panel_new(
		0, nube_config.left_panel.position,
		-1, 0,

		&nube_config.left_panel
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

	add_opacity_transition(nube.show_trans, nube.stage, nube_config.show_time, 255);
	add_slide_transition(nube.show_trans, nube.top_panel, nube_config.show_time, "shown_point");
	add_slide_transition(nube.show_trans, nube.right_panel, nube_config.show_time, "shown_point");
	add_slide_transition(nube.show_trans, nube.bottom_panel, nube_config.show_time, "shown_point");
	add_slide_transition(nube.show_trans, nube.left_panel, nube_config.show_time, "shown_point");

	clutter_timeline_start(CLUTTER_TIMELINE(nube.show_trans));
}

void hide_done() {
	clutter_actor_hide(nube.stage);
}

void hide_stage() {
	clutter_timeline_stop(CLUTTER_TIMELINE(nube.show_trans));
	clutter_timeline_stop(CLUTTER_TIMELINE(nube.hide_trans));
	clutter_transition_group_remove_all(CLUTTER_TRANSITION_GROUP(nube.hide_trans));

	add_opacity_transition(nube.hide_trans, nube.stage, nube_config.hide_time, 0);
	add_slide_transition(nube.hide_trans, nube.top_panel, nube_config.hide_time, "hidden_point");
	add_slide_transition(nube.hide_trans, nube.right_panel, nube_config.hide_time, "hidden_point");
	add_slide_transition(nube.hide_trans, nube.bottom_panel, nube_config.hide_time, "hidden_point");
	add_slide_transition(nube.hide_trans, nube.left_panel, nube_config.hide_time, "hidden_point");

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
	if (clutter_init(&argc, &argv) != CLUTTER_INIT_SUCCESS) return 1;

	nube_sources_init();
	nube_widget_types_init();
	if (!nube_config_load()) return 1;
	nube_sources_start(nube_config.referenced_sources);

	nube.show_trans = clutter_transition_group_new();
	clutter_timeline_set_duration(CLUTTER_TIMELINE(nube.show_trans), nube_config.show_time);
	nube.hide_trans = clutter_transition_group_new();
	clutter_timeline_set_duration(CLUTTER_TIMELINE(nube.hide_trans), nube_config.hide_time);
	g_signal_connect(nube.hide_trans, "completed", G_CALLBACK(hide_done), NULL);

	nube.stage = clutter_stage_new();
	clutter_stage_set_use_alpha(CLUTTER_STAGE(nube.stage), TRUE);
	clutter_actor_set_background_color(nube.stage, nube_config.background);
	g_signal_connect(nube.stage, "destroy", clutter_main_quit, NULL);
	g_signal_connect(nube.stage, "fullscreen", G_CALLBACK(setup_stage), NULL);

	clutter_stage_set_fullscreen(CLUTTER_STAGE(nube.stage), TRUE);
	clutter_actor_set_opacity(nube.stage, 0);
	clutter_actor_show(nube.stage);

	Display *display = clutter_x11_get_default_display();
	Window root_window = DefaultRootWindow(display); 

	XGrabKey(
		display,
		XKeysymToKeycode(display, nube_config.show_keysym),
		AnyModifier,
		root_window,
		false,
		GrabModeAsync,
		GrabModeAsync
	);
	clutter_x11_add_filter((ClutterX11FilterFunc) filter_func, &root_window);

	clutter_threads_add_timeout(nube_config.update_delay, nube_update_widgets, NULL);

	clutter_main();

	return 0;
}
