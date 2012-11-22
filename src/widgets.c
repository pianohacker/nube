
#include <clutter/clutter.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, const NubeWidgetOptions *opts) {
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
			width * .2, height * ((energy_used) / energy_full),
			width * .6, height * (MIN(power_now, energy_now) / energy_full)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			0, height * ((energy_used + power_now) / energy_full),
			width, height * (MAX(0, energy_now - power_now) / energy_full)
		);
		cairo_fill(cr);
	} else if (strcmp(status, "Charging") == 0 || strcmp(status, "Full") == 0) {
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

void nube_update_widget(ClutterActor *widget, const NubeWidgetOptions *opts) {
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

void nube_add_widget(ClutterLayoutManager *layout, const NubeWidgetOptions *opts) {
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
			g_signal_connect(content, "draw", G_CALLBACK(nube_draw_battery), (NubeWidgetOptions*) opts);
			g_signal_connect(widget, "notify::allocation", G_CALLBACK(canvas_resize), content);
			clutter_actor_set_content(widget, content);
			break;
		case WIDGET_END:
		default:
			return;
	}

	g_object_set_data(G_OBJECT(widget), "widget_options", (NubeWidgetOptions*) opts);
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
		const NubeWidgetOptions *opts = g_object_get_data(G_OBJECT(child), "widget_options");
		if (opts != NULL) nube_update_widget(child, opts);
	}
}
