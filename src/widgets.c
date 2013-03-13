#include <clutter/clutter.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "sys-info.h"
#include "util.h"

#define _cairo_clear(cr) cairo_save(cr); cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR); cairo_paint(cr); cairo_restore(cr)

static GData *widget_types = NULL;

typedef struct {
	ClutterActor* (*init_func)(const NubeWidgetConfig *widget_config);
	void (*draw_func)(ClutterActor *actor, const NubeWidgetConfig *widget_config);
} _NubeWidgetType;

void nube_draw_battery(ClutterActor *actor, cairo_t *cr, gint width, gint height, const NubeWidgetConfig *widget_config) {
	double energy, power;
	nube_sys_get_power(&energy, &power);
	double energy_used = 1 - energy;
	ClutterColor partial_color = WIDGET_PARTIAL_COLOR;

	_cairo_clear(cr);

	if (power < 0) {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * energy_used
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, nube_config.fg);

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

		clutter_cairo_set_source_color(cr, nube_config.fg);

		cairo_rectangle(cr,
			0, height * energy_used,
			width, height * energy
		);
		cairo_fill(cr);
	}
}

void nube_draw_cpu(ClutterActor *actor, cairo_t *cr, gint width, gint height, const NubeWidgetConfig *widget_config) {
	double usage = nube_sys_get_cpu();
	_cairo_clear(cr);

	clutter_cairo_set_source_color(cr, nube_config.fg);

	g_debug("Drawing %g within %d, %d", usage, width, height);

	cairo_rectangle(cr,
		0, height * (1 - usage),
		width, height * usage
	);
	cairo_fill(cr);
}

void nube_update_widget(ClutterActor *widget, const NubeWidgetConfig *widget_config) {
	/*char **config_parts = g_strsplit_set(opts->config, "\n", -1);
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
	}*/
}

static void _canvas_resize_cb(ClutterActor *actor, GParamSpec *spec, ClutterCanvas *canvas) {
	clutter_canvas_set_size(canvas, clutter_actor_get_width(actor), clutter_actor_get_height(actor));
}

/*static ClutterActor* _margined_canvas_new(int margin, GCallback draw_cb, const NubeWidgetOptions *opts) {
	ClutterActor *widget = clutter_actor_new();

	ClutterMargin margins;
	margins.top = margins.right = margins.bottom = margins.left = margin;
	clutter_actor_set_margin(widget, &margins);

	ClutterContent *content = clutter_canvas_new();
	g_signal_connect(content, "draw", draw_cb, (NubeWidgetOptions*) opts);
	g_signal_connect(widget, "notify::allocation", G_CALLBACK(_canvas_resize_cb), content);
	clutter_actor_set_content(widget, content);

	return widget;
}*/

ClutterActor* _text_init(const NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_text_new();
	clutter_text_set_single_line_mode(CLUTTER_TEXT(widget), TRUE);
	clutter_text_set_color(CLUTTER_TEXT(widget), nube_config.fg);

	return widget;
}

void _text_draw(ClutterActor *actor, const NubeWidgetConfig *widget_config) {
}

ClutterActor* _vertical_bar_init(const NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_text_new();

	return widget;
}

void _vertical_bar_draw(ClutterActor *actor, const NubeWidgetConfig *widget_config) {
}

void _widget_add_property(GQuark prop_id, GValue *value, ClutterActor *widget) {
	GObjectClass *klass = G_OBJECT_GET_CLASS(G_OBJECT(widget));
	GParamSpec *param_spec;
	const gchar *raw_prop = g_quark_to_string(prop_id);
	gchar prop[strlen(raw_prop)+1];
	int i;

	for (i = 0; raw_prop[i]; i++) {
		if (raw_prop[i] == '_') {
			prop[i] = '-';
		} else {
			prop[i] = raw_prop[i];
		}
	}

	prop[i] = '\0';

	if (strcmp(prop, "position") == 0 || strcmp(prop, "pivot-point") == 0) {
		g_object_set(G_OBJECT(widget), prop, value, NULL);
	} else if ((param_spec = g_object_class_find_property(klass, prop))) {
		g_debug("Setting %s.%s to %s:%s", G_OBJECT_TYPE_NAME(widget), prop, g_type_name(G_VALUE_TYPE(value)), g_strdup_value_contents(value));

		if (G_VALUE_TYPE(value) != param_spec->value_type && !g_value_type_transformable(G_VALUE_TYPE(value), param_spec->value_type)) {
			g_printerr("Property %s.%s expects %s, got %s\n",
				G_OBJECT_TYPE_NAME(widget),
				prop,
				g_type_name(param_spec->value_type),
				g_type_name(G_VALUE_TYPE(value))
			);
			exit(1);
		}

		g_object_set_property(G_OBJECT(widget), prop, value);
	} else {
		g_debug("Unknown property %s.%s found while creating widget", G_OBJECT_TYPE_NAME(widget), prop);
		return;
	}
}

void nube_widget_add(ClutterActor *panel, const NubeWidgetConfig *widget_config) {
	_NubeWidgetType *widget_type = g_datalist_id_get_data(&widget_types, widget_config->type_id);

	if (widget_type == NULL) {
		g_printerr("Unknown widget type: %s\n", g_quark_to_string(widget_config->type_id));
		exit(1);
	}

	ClutterActor *widget = widget_type->init_func(widget_config);

	g_datalist_foreach((GData**) &widget_config->props, (GDataForeachFunc) _widget_add_property, widget);

	g_object_set_data(G_OBJECT(widget), "widget_options", (NubeWidgetConfig*) widget_config);
	/*nube_update_widget(widget, opts);*/
}

void nube_widget_type_register(
		const gchar *name,
		ClutterActor* (*init_func)(const NubeWidgetConfig *widget_config),
		void (*draw_func)(ClutterActor *actor, const NubeWidgetConfig *widget_config)
	) {
	_NubeWidgetType *type_entry = g_new0(_NubeWidgetType, 1);
	type_entry->init_func = init_func;
	type_entry->draw_func = draw_func;

	g_datalist_set_data(&widget_types, name, type_entry);
}

void nube_widget_types_init() {
	nube_widget_type_register("text", _text_init, _text_draw);
	nube_widget_type_register("vertical_bar", _vertical_bar_init, _vertical_bar_draw);
}

void nube_update_all(ClutterActor *container) {
	ClutterActor *child;
	ClutterActorIter iter;

	clutter_actor_iter_init(&iter, container);
	while (clutter_actor_iter_next(&iter, &child)) {
		/*const NubeWidgetOptions *opts = g_object_get_data(G_OBJECT(child), "widget_options");
		if (opts != NULL) nube_update_widget(child, opts);*/
	}
}
