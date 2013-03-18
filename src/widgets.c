#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "sources.h"
#include "sys-info.h"
#include "util.h"

#define _cairo_clear(cr) cairo_save(cr); cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR); cairo_paint(cr); cairo_restore(cr)

static GData *widget_types = NULL;

typedef struct {
	ClutterActor* (*init_func)(NubeWidgetConfig *widget_config);
	void (*draw_func)(ClutterActor *actor, NubeWidgetConfig *widget_config);
} _NubeWidgetType;

void _vertical_bar_render(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeWidgetConfig *widget_config) {
	double value, change_over_hour = 0;
	nube_source_get(widget_config->source_id, "value", &value);
	nube_source_get(widget_config->source_id, "change_over_hour", &change_over_hour);
	double value_left = 1 - value;
	ClutterColor partial_color = WIDGET_PARTIAL_COLOR;

	_cairo_clear(cr);

	if (change_over_hour < 0) {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * value_left
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, nube_config.fg);

		cairo_rectangle(cr,
			width * .3, height * value_left,
			width * .4, height * MIN(-change_over_hour, value)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			0, height * (value_left - change_over_hour),
			width, height * MAX(0, value + change_over_hour)
		);
		cairo_fill(cr);
	} else {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * MAX(0, value_left - change_over_hour)
		);
		cairo_fill(cr);

		cairo_rectangle(cr,
			width * .3, height * MAX(0, value_left - change_over_hour),
			width * .4, height * MIN(change_over_hour, value_left)
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, nube_config.fg);

		cairo_rectangle(cr,
			0, height * value_left,
			width, height * value
		);
		cairo_fill(cr);
	}
}

void nube_widget_update(ClutterActor *widget, const _NubeWidgetType *widget_type, NubeWidgetConfig *widget_config) {
	if (widget_type->draw_func) widget_type->draw_func(widget, widget_config);
}

static void _canvas_resize_cb(ClutterActor *actor, GParamSpec *spec, ClutterCanvas *canvas) {
	clutter_canvas_set_size(canvas, clutter_actor_get_width(actor), clutter_actor_get_height(actor));
}

static ClutterActor* _canvas_widget_new(GCallback draw_cb, NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_actor_new();

	ClutterContent *content = clutter_canvas_new();
	g_signal_connect(content, "draw", draw_cb, (NubeWidgetConfig*) widget_config);
	g_signal_connect(widget, "notify::allocation", G_CALLBACK(_canvas_resize_cb), content);
	clutter_actor_set_content(widget, content);

	return widget;
}

ClutterActor* _text_init(NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_text_new();
	clutter_text_set_single_line_mode(CLUTTER_TEXT(widget), TRUE);
	clutter_text_set_color(CLUTTER_TEXT(widget), nube_config.fg);

	return widget;
}

void _text_draw(ClutterActor *widget, NubeWidgetConfig *widget_config) {
}

ClutterActor* _vertical_bar_init(NubeWidgetConfig *widget_config) {
	ClutterActor *widget = _canvas_widget_new(G_CALLBACK(_vertical_bar_render), widget_config);

	return widget;
}

void _vertical_bar_draw(ClutterActor *widget, NubeWidgetConfig *widget_config) {
	clutter_content_invalidate(clutter_actor_get_content(widget));
}

ClutterActor* _icon_init(NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_actor_new();
	char image_filename[PATH_MAX];

	sprintf(image_filename, "%s/.nube/icons/%s", getenv("HOME"), g_value_get_string(g_datalist_get_data(&widget_config->props, "file")));

	GError *err = NULL;
	ClutterContent *content = clutter_image_new();
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_filename, &err);

	if (err) {
		g_printerr("%s\n", err->message);
		exit(1);
	}

	if (!err) {
		clutter_image_set_data (CLUTTER_IMAGE(content),
				gdk_pixbuf_get_pixels(pixbuf),
				gdk_pixbuf_get_has_alpha(pixbuf)
				? COGL_PIXEL_FORMAT_RGBA_8888
				: COGL_PIXEL_FORMAT_RGB_888,
				gdk_pixbuf_get_width(pixbuf),
				gdk_pixbuf_get_height(pixbuf),
				gdk_pixbuf_get_rowstride(pixbuf),
				&err);
	}

	if (err) {
		g_printerr("Could not load icon file %s: %s\n", image_filename, err->message);
		exit(1);
	}

	clutter_actor_set_content(widget, content);
	g_object_unref(content);

	clutter_actor_set_size(
		widget,
		gdk_pixbuf_get_width(pixbuf),
		gdk_pixbuf_get_height(pixbuf)
	);

	return widget;
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

	if ((param_spec = g_object_class_find_property(klass, prop))) {
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

void nube_widget_add(ClutterActor *panel, NubeWidgetConfig *widget_config) {
	_NubeWidgetType *widget_type = g_datalist_id_get_data(&widget_types, widget_config->type_id);

	if (widget_type == NULL) {
		g_printerr("Unknown widget type: %s\n", g_quark_to_string(widget_config->type_id));
		exit(1);
	}

	ClutterActor *widget = widget_type->init_func(widget_config);

	g_datalist_foreach((GData**) &widget_config->props, (GDataForeachFunc) _widget_add_property, widget);

	if (widget_config->position) {
		clutter_actor_set_position(widget, widget_config->position->x + nube_config.glow_size, widget_config->position->y + nube_config.glow_size);
	}

	if (widget_config->pivot_point) {
		clutter_actor_set_pivot_point(widget, widget_config->pivot_point->x, widget_config->pivot_point->y);
	}

	g_object_set_data(G_OBJECT(widget), "widget_type", widget_type);
	g_object_set_data(G_OBJECT(widget), "widget_config", (NubeWidgetConfig*) widget_config);

	clutter_actor_add_child(panel, widget);
	nube_widget_update(widget, widget_type, widget_config);
}

void nube_widget_type_register(
		const gchar *name,
		ClutterActor* (*init_func)(NubeWidgetConfig *widget_config),
		void (*draw_func)(ClutterActor *actor, NubeWidgetConfig *widget_config)
	) {
	_NubeWidgetType *type_entry = g_new0(_NubeWidgetType, 1);
	type_entry->init_func = init_func;
	type_entry->draw_func = draw_func;

	g_datalist_set_data(&widget_types, name, type_entry);
}

void nube_widget_types_init() {
	nube_widget_type_register("icon", _icon_init, NULL);
	nube_widget_type_register("text", _text_init, _text_draw);
	nube_widget_type_register("vertical_bar", _vertical_bar_init, _vertical_bar_draw);
}

void nube_update_all(ClutterActor *container) {
	ClutterActor *child;
	ClutterActorIter iter;

	clutter_actor_iter_init(&iter, container);
	while (clutter_actor_iter_next(&iter, &child)) {
		const _NubeWidgetType *widget_type = g_object_get_data(G_OBJECT(child), "widget_type");
		if (widget_type == NULL) continue;
		NubeWidgetConfig *widget_config = g_object_get_data(G_OBJECT(child), "widget_config");

		nube_widget_update(child, widget_type, widget_config);
	}
}
