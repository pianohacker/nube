#include <cairo.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <gdk/gdk.h>
#include <glib-object.h>
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include "config.h"

struct {
	gfloat width;
	gfloat height;

	ClutterActor *stage;
	ClutterActor *top_panel;
	ClutterActor *right_panel;
	ClutterActor *bottom_panel;
	ClutterActor *left_panel;
} nube;

typedef struct {
	double x;
	double y;
} DPoint;

char *runtime_path;
char* nube_get_runtime_path(char *argv0) {
        char* dir = realpath(dirname(argv0), NULL);

        if (strlen(dir) >= 4 && strcmp(dir + strlen(dir) - 4, "/bin") == 0) {
                dir[strlen(dir) - 4] = 0;
        }

        return dir;
}

cairo_pattern_t* nube_offset_quads(cairo_path_t *path, double offset) {
	cairo_pattern_t *result = cairo_pattern_create_mesh();
	cairo_path_data_t *cur_data = path->data;
	cairo_path_data_t *end = path->data + path->num_data;

	int path_len = path->num_data / 2;
	double *norm_angles = malloc(sizeof(double) * path_len);
	double *corner_angles = malloc(sizeof(double) * path_len);
	DPoint *points = calloc(sizeof(DPoint), path_len);

	for (; cur_data < end; cur_data++) {
		if (cur_data->header.type != CAIRO_PATH_MOVE_TO) continue;

		int num_points = 1;
		points[0].x = cur_data[1].point.x;
		points[0].y = cur_data[1].point.y;
		
		cairo_path_data_t *path_data = cur_data + 2;
		int i = 1;
		bool in_path = true;
		for (; path_data < end && in_path; path_data++) {
			double x, y;
			if (path_data->header.type == CAIRO_PATH_LINE_TO) {
				points[i].x = x = path_data[1].point.x;
				points[i].y = y = path_data[1].point.y;
				num_points++;
				path_data++;
			} else if (path_data->header.type == CAIRO_PATH_CLOSE_PATH) {
				x = cur_data[1].point.x;
				y = cur_data[1].point.y;
				path_data++;
				in_path = false;
			} else {
				path_data += path_data->header.length;
				continue;
			}

			norm_angles[i - 1] = atan2(
				y - points[i - 1].y,
				x - points[i - 1].x
			) - M_PI / 2;

			i++;
		}
		cur_data = path_data;

		for (i = 0; i < num_points; i++) {
			double angle1 = norm_angles[(i - 1 + num_points) % num_points];
			double angle2 = norm_angles[i];
			if (angle2 < angle1) angle2 += M_PI * 2;

			corner_angles[i] = angle1 + (angle2 - angle1) / 2.;
		}

		for (i = 0; i < num_points; i++) {
			int next_i = (i + 1) % num_points;
			cairo_mesh_pattern_begin_patch(result);
			cairo_mesh_pattern_move_to(result, points[next_i].x, points[next_i].y);
			cairo_mesh_pattern_line_to(result, points[i].x, points[i].y);
			cairo_mesh_pattern_line_to(result, points[i].x + cos(corner_angles[i]) * GLOW_SIZE, points[i].y + sin(corner_angles[i]) * GLOW_SIZE);
			cairo_mesh_pattern_line_to(result, points[next_i].x + cos(corner_angles[next_i]) * GLOW_SIZE, points[next_i].y + sin(corner_angles[next_i]) * GLOW_SIZE);

			cairo_mesh_pattern_set_corner_color_rgba(result, 0, 1, 1, 1, 0.4);
			cairo_mesh_pattern_set_corner_color_rgba(result, 1, 1, 1, 1, 0.4);
			cairo_mesh_pattern_set_corner_color_rgba(result, 2, 1, 1, 1, 0);

			cairo_mesh_pattern_set_corner_color_rgba(result, 3, 1, 1, 1, 0);
			cairo_mesh_pattern_end_patch(result);
		}
	}

	return result;
}xxxxxxxxvcccccccc

void nube_draw_cut_rect(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeCutRectOptions *opts) {
	width -= GLOW_SIZE * 2;
	height -= GLOW_SIZE * 2;

	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	ccccccccx

	cairo_translate(cr, GLOW_SIZE, GLOW_SIZE);

	cairo_move_to(cr, 0, opts->top_left);
	cairo_line_to(cr, opts->top_left, 0);

	cairo_line_to(cr, width - opts->top_right, 0);
	cairo_line_to(cr, width, opts->top_right);

	cairo_line_to(cr, width, height - opts->bottom_right);
	cairo_line_to(cr, width - opts->bottom_right, height);

	cairo_line_to(cr, opts->bottom_left, height);
	cairo_line_to(cr, 0, height - opts->bottom_left);

	cairo_close_path(cr);
	cairo_set_source(cr, nube_offset_quads(cairo_copy_path(cr), GLOW_SIZE));
	cairo_paint(cr);

	clutter_cairo_set_source_color(cr, &opts->fill_color);
	cairo_fill_preserve(cr);

	/*clutter_cairo_set_source_color(cr, &opts->stroke_color);*/
	/*cairo_set_line_width(cr, opts->border_width);*/
	/*cairo_stroke(cr);*/
}

ClutterActor* setup_panel(int x, int y, int width, int height, NubeCutRectOptions options) {
	ClutterActor *panel = clutter_actor_new();
	ClutterContent *content = clutter_canvas_new();

	clutter_actor_set_size(panel, width + GLOW_SIZE * 2, height + GLOW_SIZE * 2);
	clutter_actor_set_position(panel, x - GLOW_SIZE, y - GLOW_SIZE);
	clutter_actor_add_child(CLUTTER_ACTOR(nube.stage), panel);

	g_signal_connect(content, "draw", G_CALLBACK(nube_draw_cut_rect), &options);
	clutter_canvas_set_size(CLUTTER_CANVAS(content), width + GLOW_SIZE * 2, height + GLOW_SIZE * 2);
	clutter_actor_set_content(panel, content);

	return panel;
}

void setup_stage(ClutterStage *stage, gpointer *data) {
	clutter_actor_get_size(CLUTTER_ACTOR(stage), &nube.width, &nube.height);

	nube.top_panel = setup_panel(
		0, -TOP_PANEL_OPTIONS.border_width * 2,
		nube.width, TOP_PANEL_OPTIONS.size,
		TOP_PANEL_OPTIONS
	);

	nube.right_panel = setup_panel(
		nube.width - RIGHT_PANEL_OPTIONS.size + RIGHT_PANEL_OPTIONS.border_width * 2, GUTTER_SIZE + TOP_PANEL_OPTIONS.size,
		RIGHT_PANEL_OPTIONS.size, nube.height - TOP_PANEL_OPTIONS.size - BOTTOM_PANEL_OPTIONS.size - GUTTER_SIZE * 2,
		RIGHT_PANEL_OPTIONS
	);

	nube.bottom_panel = setup_panel(
		0, nube.height - BOTTOM_PANEL_OPTIONS.size + BOTTOM_PANEL_OPTIONS.border_width * 2,
		nube.width, BOTTOM_PANEL_OPTIONS.size,
		BOTTOM_PANEL_OPTIONS
	);

	nube.left_panel = setup_panel(
		-LEFT_PANEL_OPTIONS.border_width * 2, GUTTER_SIZE + TOP_PANEL_OPTIONS.size,
		LEFT_PANEL_OPTIONS.size, nube.height -TOP_PANEL_OPTIONS.size - BOTTOM_PANEL_OPTIONS.size - GUTTER_SIZE * 2,
		LEFT_PANEL_OPTIONS
	);
}

ClutterX11FilterReturn filter_func(XEvent *xev, ClutterEvent *cev, Window *root_window) {
	if (xev->xany.window == *root_window) {
		switch (xev->xany.type) {
			case KeyPress:
				clutter_actor_show(nube.stage);
				break;
			case KeyRelease:
				clutter_actor_hide(nube.stage);
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

	ClutterColor stage_color = BACK_COLOR;

	nube.stage = clutter_stage_new();
	clutter_stage_set_use_alpha(CLUTTER_STAGE(nube.stage), TRUE);
	clutter_stage_set_color(CLUTTER_STAGE(nube.stage), &stage_color);
	g_signal_connect(nube.stage, "destroy", clutter_main_quit, NULL);
	g_signal_connect(nube.stage, "fullscreen", G_CALLBACK(setup_stage), NULL);

	clutter_stage_set_fullscreen(CLUTTER_STAGE(nube.stage), TRUE);

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

	clutter_main();

	return 0;
}
