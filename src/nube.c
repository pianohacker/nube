#include <cairo.h>
#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>
#include <fcntl.h>
#include <gdk/gdk.h>
#include <glib-object.h>
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <X11/Xlib.h>

#include "config.h"

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

double nube_get_num_file(const char *path) {
	int fd = open(path, O_RDONLY);
	char buffer[64];
	buffer[read(fd, buffer, 64)] = '\0';
	
	return strtod(buffer, NULL);
}

char* nube_get_str_file(const char *path) {
	int fd = open(path, O_RDONLY);
	char *buffer = malloc(64);
	buffer[read(fd, buffer, 64) - 1] = '\0';
	
	return buffer;
}

cairo_pattern_t* nube_offset_quads(cairo_path_t *path, double offset) {
	cairo_pattern_t *result = cairo_pattern_create_mesh();
	cairo_path_data_t *cur_data = path->data;
	cairo_path_data_t *end = path->data + path->num_data;

	int path_len = path->num_data / 2;
	double *norm_angles = malloc(sizeof(double) * path_len);
	double *angle_spans = malloc(sizeof(double) * path_len);
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
			double diff = angle2 - angle1;

			corner_angles[i] = angle1 + diff / 2.;
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

	free(norm_angles);
	free(corner_angles);
	free(points);

	return result;
}

void nube_draw_cut_rect(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubePanelOptions *opts) {
	width -= GLOW_SIZE * 2;
	height -= GLOW_SIZE * 2;

	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_restore(cr);

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

void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeWidgetOptions *opts) {
	double energy_full = nube_get_num_file(BATTERY_PREFIX "/energy_full");
	double energy_now = nube_get_num_file(BATTERY_PREFIX "/energy_now");
	double power_now = nube_get_num_file(BATTERY_PREFIX "/power_now");
	double energy_used = energy_full - energy_now;
	char* status = nube_get_str_file(BATTERY_PREFIX "/status");
	ClutterColor fg_color = FG_COLOR;
	ClutterColor partial_color = WIDGET_PARTIAL_COLOR;

	printf("Drawing \"%s\": %g/%g - %g within %d, %d\n", status, energy_now / 1000000, energy_full / 1000000, power_now / 1000000, width, height);

	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_restore(cr);

	if (strcmp(status, "Discharging") == 0) {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * (energy_used / energy_full)
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, &fg_color);

		cairo_rectangle(cr,
			width * .25, height * ((energy_used) / energy_full),
			width * .5, height * (MIN(power_now, energy_now) / energy_full)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			0, height * ((energy_used + power_now) / energy_full),
			width, height * (MAX(0, energy_now - power_now) / energy_full)
		);
		cairo_fill(cr);
	} else if (strcmp(status, "Charging") == 0) {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * (MAX(0, energy_used - power_now) / energy_full)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			width * .25, height * (MAX(0, energy_used - power_now) / energy_full),
			width * .5, height * (MIN(power_now, energy_used) / energy_full)
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, &fg_color);

		cairo_rectangle(cr,
			0, height * (energy_used / energy_full),
			width, height * (energy_now / energy_full)
		);
		cairo_fill(cr);
	}

	free(status);
}

void canvas_resize(ClutterActor *actor, GParamSpec *spec, ClutterCanvas *canvas) {
	clutter_canvas_set_size(canvas, clutter_actor_get_width(actor), clutter_actor_get_height(actor));
}

void nube_update_widget(ClutterActor *widget, NubeWidgetOptions *opts) {
	char **config_parts = g_strsplit_set(opts->config, "\n", -1);
	char buffer[64];
	time_t local;

	switch (opts->type) {
		case WIDGET_CLOCK:
			time(&local);
			strftime(buffer, 64, config_parts[0], localtime(&local));
			clutter_text_set_text(CLUTTER_TEXT(widget), buffer);
			break;
		case WIDGET_BATTERY:
			clutter_content_invalidate(clutter_actor_get_content(widget));
			break;
		case WIDGET_END:
		default:
			return;
	}
}

void nube_add_widget(ClutterLayoutManager *layout, NubeWidgetOptions *opts) {
	ClutterActor *widget;
	ClutterColor fg_color = FG_COLOR;
	char **config_parts = g_strsplit_set(opts->config, "\n", -1);

	switch (opts->type) {
		case WIDGET_CLOCK:
			widget = clutter_text_new();
			clutter_text_set_font_name(CLUTTER_TEXT(widget), config_parts[1] ? config_parts[1] : DEFAULT_FONT);
			clutter_text_set_color(CLUTTER_TEXT(widget), &fg_color);
			clutter_text_set_single_line_mode(CLUTTER_TEXT(widget), TRUE);
			break;
		case WIDGET_BATTERY:
			widget = clutter_actor_new();

			ClutterMargin margins;
			margins.top = margins.right = margins.bottom = margins.left = strtod(config_parts[1], NULL);
			clutter_actor_set_margin(widget, &margins);

			g_object_set(widget, "natural-width", strtod(config_parts[0], NULL), NULL);
			clutter_actor_set_request_mode(widget, CLUTTER_REQUEST_HEIGHT_FOR_WIDTH);

			ClutterContent *content = clutter_canvas_new();
			g_signal_connect(content, "draw", G_CALLBACK(nube_draw_battery), opts);
			g_signal_connect(widget, "notify::allocation", G_CALLBACK(canvas_resize), content);
			clutter_actor_set_content(widget, content);
			break;
		case WIDGET_END:
		default:
			return;
	}

	g_object_set_data(G_OBJECT(widget), "widget_options", opts);
	clutter_actor_set_x_expand(widget, TRUE);
	clutter_actor_set_y_expand(widget, TRUE);
	clutter_actor_set_x_align(widget, opts->x_align);
	clutter_actor_set_y_align(widget, opts->y_align);
	clutter_grid_layout_attach(
		CLUTTER_GRID_LAYOUT(layout), widget,
		opts->column,
		opts->row,
		opts->width,
		opts->height
	);
	nube_update_widget(widget, opts);
}

void nube_update_all(ClutterActor *container) {
	ClutterActor *child;
	ClutterActorIter iter;

	clutter_actor_iter_init(&iter, clutter_actor_get_first_child(container));
	while (clutter_actor_iter_next(&iter, &child)) {
		NubeWidgetOptions *opts = g_object_get_data(G_OBJECT(child), "widget_options");
		if (opts == NULL) continue;

		printf("Updating %p\n", child);
		nube_update_widget(child, opts);
	}
}

gboolean nube_update_widgets(gpointer user_data) {
	nube_update_all(nube.top_panel);
	nube_update_all(nube.right_panel);
	nube_update_all(nube.bottom_panel);
	nube_update_all(nube.left_panel);

	return TRUE;
}

ClutterActor* setup_panel(gfloat x, gfloat y, gfloat hidden_x, gfloat hidden_y, gfloat width, gfloat height, NubePanelOptions *options) {
	ClutterActor *panel = clutter_actor_new();
	clutter_actor_set_size(panel, width + GLOW_SIZE * 2, height + GLOW_SIZE * 2);
	clutter_actor_set_position(panel, hidden_x, hidden_y);
	clutter_actor_add_child(CLUTTER_ACTOR(nube.stage), panel);

	ClutterActor *inner = clutter_actor_new();
	ClutterLayoutManager *layout = clutter_grid_layout_new();
	clutter_actor_set_layout_manager(inner, layout);
	clutter_actor_set_position(inner, options->left_padding + GLOW_SIZE, options->top_padding + GLOW_SIZE);
	clutter_actor_set_size(inner, width - options->left_padding - options->right_padding, height - options->top_padding - options->bottom_padding);
	clutter_actor_add_child(panel, inner);

	for (int i = 0; i < MAX_WIDGETS_SIZE && options->widget_opts[i].type; i++) {
		nube_add_widget(layout, &options->widget_opts[i]);
	}

	ClutterPoint *shown_point = clutter_point_alloc();
	shown_point->x = x;
	shown_point->y = y;
	g_object_set_data(G_OBJECT(panel), "shown_point", shown_point);

	ClutterPoint *hidden_point = clutter_point_alloc();
	hidden_point->x = hidden_x;
	hidden_point->y = hidden_y;
	g_object_set_data(G_OBJECT(panel), "hidden_point", hidden_point);

	ClutterContent *content = clutter_canvas_new();
	g_signal_connect(content, "draw", G_CALLBACK(nube_draw_cut_rect), options);
	clutter_canvas_set_size(CLUTTER_CANVAS(content), width + GLOW_SIZE * 2, height + GLOW_SIZE * 2);
	clutter_actor_set_content(panel, content);

	return panel;
}

void setup_stage(ClutterStage *stage, gpointer *data) {
	clutter_actor_get_size(CLUTTER_ACTOR(stage), &nube.width, &nube.height);

	nube.top_panel = setup_panel(
		-GLOW_SIZE, -GLOW_SIZE,
		-GLOW_SIZE, -GLOW_SIZE * 2 - TOP_PANEL_OPTIONS.size,
		nube.width, TOP_PANEL_OPTIONS.size,

		&TOP_PANEL_OPTIONS
	);

	nube.right_panel = setup_panel(
		nube.width - RIGHT_PANEL_OPTIONS.size - GLOW_SIZE, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		nube.width, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		RIGHT_PANEL_OPTIONS.size, nube.height - TOP_PANEL_OPTIONS.size - BOTTOM_PANEL_OPTIONS.size - GUTTER_SIZE * 2,

		&RIGHT_PANEL_OPTIONS
	);

	nube.bottom_panel = setup_panel(
		-GLOW_SIZE, nube.height - BOTTOM_PANEL_OPTIONS.size - GLOW_SIZE,
		-GLOW_SIZE, nube.height,
		nube.width, BOTTOM_PANEL_OPTIONS.size,

		&BOTTOM_PANEL_OPTIONS
	);

	nube.left_panel = setup_panel(
		-GLOW_SIZE, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		-GLOW_SIZE * 2 - LEFT_PANEL_OPTIONS.size, GUTTER_SIZE - GLOW_SIZE + TOP_PANEL_OPTIONS.size,
		LEFT_PANEL_OPTIONS.size, nube.height -TOP_PANEL_OPTIONS.size - BOTTOM_PANEL_OPTIONS.size - GUTTER_SIZE * 2,

		&LEFT_PANEL_OPTIONS
	);

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

	clutter_threads_add_timeout(1000, nube_update_widgets, NULL);

	clutter_main();

	return 0;
}
