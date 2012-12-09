#include <clutter/clutter.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "sys-info.h"
#include "util.h"

#define _cairo_clear(cr) cairo_save(cr); cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR); cairo_paint(cr); cairo_restore(cr)

void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, const NubeWidgetOptions *opts) {
	double energy, power;
	nube_sys_get_power(&energy, &power);
	double energy_used = 1 - energy;
	ClutterColor fg_color = FG_COLOR;
	ClutterColor partial_color = WIDGET_PARTIAL_COLOR;

	_cairo_clear(cr);

	if (power < 0) {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * energy_used
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, &fg_color);

		cairo_rectangle(cr,
			width * .3, height * energy_used,
			width * .4, height * MIN(-power, energy)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			0, height * (energy_used - power),
			width, height * MAX(0, energy + power)
		);
		cairo_fill(cr);
	} else {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * MAX(0, energy_used - power)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			width * .3, height * MAX(0, energy_used - power),
			width * .4, height * MIN(power, energy_used)
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, &fg_color);

		cairo_rectangle(cr,
			0, height * energy_used,
			width, height * energy
		);
		cairo_fill(cr);
	}
}

void nube_draw_cpu(ClutterActor *actor, cairo_t *cr, gint width, gint height, const NubeWidgetOptions *opts) {
	double usage = nube_sys_get_cpu();
	ClutterColor fg_color = FG_COLOR;

	_cairo_clear(cr);

	clutter_cairo_set_source_color(cr, &fg_color);

	g_debug("Drawing %g within %d, %d", usage, width, height);

	cairo_rectangle(cr,
		0, height * (1 - usage),
		width, height * usage
	);
	cairo_fill(cr);
}

void nube_update_widget(ClutterActor *widget, const NubeWidgetOptions *opts) {
	char **config_parts = g_strsplit_set(opts->config, "\n", -1);
	char buffer[64];
	double energy, power, usage;
	time_t local;

	switch (opts->type) {
		case WIDGET_CLOCK:
			time(&local);
			strftime(buffer, 64, config_parts[0], localtime(&local));
			clutter_text_set_text(CLUTTER_TEXT(widget), buffer);
			break;
		case WIDGET_BATTERY_BAR:
		case WIDGET_CPU_BAR:
			clutter_content_invalidate(clutter_actor_get_content(widget));
			break;
		case WIDGET_BATTERY_TEXT:
			nube_sys_get_power(&energy, &power);
			sprintf(buffer, "%.0f/%.0f%%", energy * 100, fabs(power) * 100);

			clutter_text_set_text(CLUTTER_TEXT(widget), buffer);
			break;
		case WIDGET_CPU_TEXT:
			usage = nube_sys_get_cpu();
			sprintf(buffer, "%.0f", usage);

			clutter_text_set_text(CLUTTER_TEXT(widget), buffer);
			break;
		case WIDGET_END:
		default:
			return;
	}
}

static void _canvas_resize_cb(ClutterActor *actor, GParamSpec *spec, ClutterCanvas *canvas) {
	clutter_canvas_set_size(canvas, clutter_actor_get_width(actor), clutter_actor_get_height(actor));
}

static ClutterActor* _margined_canvas_new(int margin, GCallback draw_cb, const NubeWidgetOptions *opts) {
	ClutterActor *widget = clutter_actor_new();

	ClutterMargin margins;
	margins.top = margins.right = margins.bottom = margins.left = margin;
	clutter_actor_set_margin(widget, &margins);

	ClutterContent *content = clutter_canvas_new();
	g_signal_connect(content, "draw", draw_cb, (NubeWidgetOptions*) opts);
	g_signal_connect(widget, "notify::allocation", G_CALLBACK(_canvas_resize_cb), content);
	clutter_actor_set_content(widget, content);

	return widget;
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
		case WIDGET_BATTERY_BAR:
			widget = _margined_canvas_new(strtod(config_parts[1], NULL), G_CALLBACK(nube_draw_battery), opts);

			g_object_set(widget, "natural-width", strtod(config_parts[0], NULL), NULL);
			clutter_actor_set_request_mode(widget, CLUTTER_REQUEST_HEIGHT_FOR_WIDTH);
			break;
		case WIDGET_CPU_BAR:
			widget = _margined_canvas_new(strtod(config_parts[1], NULL), G_CALLBACK(nube_draw_cpu), opts);

			g_object_set(widget, "natural-width", strtod(config_parts[0], NULL), NULL);
			clutter_actor_set_request_mode(widget, CLUTTER_REQUEST_HEIGHT_FOR_WIDTH);
			break;
		case WIDGET_BATTERY_TEXT:
		case WIDGET_CPU_TEXT:
			widget = clutter_text_new();
			clutter_text_set_font_name(CLUTTER_TEXT(widget), config_parts[0] ? config_parts[0] : DEFAULT_FONT);
			clutter_text_set_color(CLUTTER_TEXT(widget), &fg_color);
			clutter_text_set_single_line_mode(CLUTTER_TEXT(widget), TRUE);
			clutter_actor_set_request_mode(widget, CLUTTER_REQUEST_WIDTH_FOR_HEIGHT);
			break;
		case WIDGET_END:
		default:
			return;
	}

	g_object_set_data(G_OBJECT(widget), "widget_options", (NubeWidgetOptions*) opts);
	clutter_actor_set_x_expand(widget, opts->x_expand);
	clutter_actor_set_y_expand(widget, opts->y_expand);
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
