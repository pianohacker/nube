#include <clutter/clutter.h>
#include <glib.h>
#include <gsdl/parser.h>
#include <gsdl/syntax.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

NubeGlobalConfig nube_config;
static GError *parse_err;

static void _root_error_handler(
		GSDLParserContext *context,
		GError *err,
		gpointer user_data
	) {
	parse_err = err;
}

static void _appearance_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	GValue *value;

	if (strcmp(name, "background") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_STRING, &value, GSDL_GTYPE_END)) return;

		nube_config.background = clutter_color_alloc();
		clutter_color_from_string(nube_config.background, g_value_get_string(value));
	} else if (strcmp(name, "fg") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_STRING, &value, GSDL_GTYPE_END)) return;

		nube_config.fg = clutter_color_alloc();
		clutter_color_from_string(nube_config.fg, g_value_get_string(value));
	} else if (strcmp(name, "glow_size") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_DOUBLE, &value, GSDL_GTYPE_END)) return;

		nube_config.glow_size = g_value_get_double(value);
	} else {
		g_set_error(
			err,
			GSDL_SYNTAX_ERROR,
			GSDL_SYNTAX_ERROR_UNEXPECTED_TAG,
			"Unexpected tag in appearance: %s",
			name
		);
		return;
	}

	g_free(value);
}

static void _appearance_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "appearance") == 0) gsdl_parser_context_pop(context);
}

static GSDLParser appearance_parser = {
	_appearance_start_tag,
	_appearance_end_tag,
	_root_error_handler
};

static void _behavior_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	GValue *value;

	if (strcmp(name, "show_time") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_INT, &value, GSDL_GTYPE_END)) return;

		nube_config.show_time = g_value_get_int(value);
	} else if (strcmp(name, "hide_time") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_INT, &value, GSDL_GTYPE_END)) return;

		nube_config.hide_time = g_value_get_int(value);
	} else if (strcmp(name, "update_delay") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_INT, &value, GSDL_GTYPE_END)) return;

		nube_config.update_delay = g_value_get_int(value);
	} else {
		g_set_error(
			err,
			GSDL_SYNTAX_ERROR,
			GSDL_SYNTAX_ERROR_UNEXPECTED_TAG,
			"Unexpected tag in behavior: %s",
			name
		);
		return;
	}

	g_free(value);
}

static void _behavior_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "behavior") == 0) gsdl_parser_context_pop(context);
}

static GSDLParser behavior_parser = {
	_behavior_start_tag,
	_behavior_end_tag,
	_root_error_handler
};

static void _panel_shape_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	GValue *x_value, *y_value;
	NubePanelConfig *panel_config = (NubePanelConfig*) user_data;

	if (strcmp(name, "content") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_DOUBLE, &x_value, G_TYPE_DOUBLE, &y_value, GSDL_GTYPE_END)) return;

		gdouble px = g_value_get_double(x_value), py = g_value_get_double(y_value);

		if (px > panel_config->shape_max_x) panel_config->shape_max_x = px;
		if (py > panel_config->shape_max_y) panel_config->shape_max_y = py;

		g_array_append_val(panel_config->shape_elems, px);
		g_array_append_val(panel_config->shape_elems, py);

		g_free(x_value);
		g_free(y_value);
	} else {
		g_set_error(
			err,
			GSDL_SYNTAX_ERROR,
			GSDL_SYNTAX_ERROR_UNEXPECTED_TAG,
			"Unexpected tag in panel shape: %s",
			name
		);
		return;
	}
}

static void _panel_shape_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "shape") == 0) gsdl_parser_context_pop(context);
}

static GSDLParser panel_shape_parser = {
	_panel_shape_start_tag,
	_panel_shape_end_tag,
	_root_error_handler
};

static void _panel_widgets_inner_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	GValue *value, *y_value;
	NubeWidgetConfig *widget_config = (NubeWidgetConfig*) user_data;

	if (strcmp(name, "position") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_FLOAT, &value, G_TYPE_FLOAT, &y_value, GSDL_GTYPE_END)) return;

		widget_config->position = clutter_point_alloc();
		clutter_point_init(widget_config->position, g_value_get_float(value), g_value_get_float(y_value));

		g_free(value);
		g_free(y_value);
	} else if (strcmp(name, "pivot_point") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_FLOAT, &value, G_TYPE_FLOAT, &y_value, GSDL_GTYPE_END)) return;

		widget_config->pivot_point = clutter_point_alloc();
		clutter_point_init(widget_config->pivot_point, g_value_get_float(value), g_value_get_float(y_value));

		g_free(value);
		g_free(y_value);
	} else {
		if (!gsdl_parser_collect_values(name, values, err, GSDL_GTYPE_ANY, &value, GSDL_GTYPE_END)) return;

		g_datalist_set_data(&widget_config->props, name, value);

		// Value not freed, as left in datalist
	}
}

static void _panel_widgets_inner_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	NubeWidgetConfig *widget_config = (NubeWidgetConfig*) user_data;

	if (g_quark_from_string(name) == widget_config->type_id) gsdl_parser_context_pop(context);
}

static GSDLParser panel_widgets_inner_parser = {
	_panel_widgets_inner_start_tag,
	_panel_widgets_inner_end_tag,
	_root_error_handler
};

static void _panel_widgets_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	GValue *value = NULL;
	NubePanelConfig *panel_config = (NubePanelConfig*) user_data;

	NubeWidgetConfig *widget_config = g_new0(NubeWidgetConfig, 1);
	g_array_append_val(panel_config->widgets, widget_config);

	widget_config->type_id = g_quark_from_string(name);
	
	gsdl_parser_collect_attributes(name, attr_names, attr_values, err, G_TYPE_STRING | GSDL_GTYPE_OPTIONAL, "source", &value, GSDL_GTYPE_END);

	if (value) {
		widget_config->source_id = g_quark_from_string(g_value_get_string(value));
		g_free(value);
	}

	g_datalist_init(&widget_config->props);

	gsdl_parser_context_push(context, &panel_widgets_inner_parser, widget_config);
}

static void _panel_widgets_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "widgets") == 0) gsdl_parser_context_pop(context);
}

static GSDLParser panel_widgets_parser = {
	_panel_widgets_start_tag,
	_panel_widgets_end_tag,
	_root_error_handler
};

static void _panel_inner_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	GValue *value;
	NubePanelConfig *panel_config = (NubePanelConfig*) user_data;

	if (strcmp(name, "position") == 0) {
		if (!gsdl_parser_collect_values(name, values, err, G_TYPE_DOUBLE, &value, GSDL_GTYPE_END)) return;

		panel_config->position = g_value_get_double(value);

		g_free(value);
	} else if (strcmp(name, "shape") == 0) {
		if (!gsdl_parser_collect_attributes(name, attr_names, attr_values, err, G_TYPE_STRING, "background", &value, GSDL_GTYPE_END)) return;

		panel_config->background = clutter_color_alloc();
		clutter_color_from_string(panel_config->background, g_value_get_string(value));

		panel_config->shape_elems = g_array_new(FALSE, FALSE, sizeof(double));

		gsdl_parser_context_push(context, &panel_shape_parser, user_data);

		g_free(value);
	} else if (strcmp(name, "widgets") == 0) {
		panel_config->widgets = g_array_new(FALSE, FALSE, sizeof(NubeWidgetConfig*));
		gsdl_parser_context_push(context, &panel_widgets_parser, user_data);
	} else {
		g_set_error(
			err,
			GSDL_SYNTAX_ERROR,
			GSDL_SYNTAX_ERROR_UNEXPECTED_TAG,
			"Unexpected tag in panel: %s",
			name
		);
	}
}

static void _panel_inner_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "top") == 0 || strcmp(name, "left") == 0 || strcmp(name, "right") == 0 || strcmp(name, "bottom") == 0) gsdl_parser_context_pop(context);
}

static GSDLParser panel_inner_parser = {
	_panel_inner_start_tag,
	_panel_inner_end_tag,
	_root_error_handler
};

static void _panels_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "top") == 0) {
		gsdl_parser_context_push(context, &panel_inner_parser, &nube_config.top_panel);
	} else if (strcmp(name, "left") == 0) {
		gsdl_parser_context_push(context, &panel_inner_parser, &nube_config.left_panel);
	} else if (strcmp(name, "right") == 0) {
		gsdl_parser_context_push(context, &panel_inner_parser, &nube_config.right_panel);
	} else if (strcmp(name, "bottom") == 0) {
		gsdl_parser_context_push(context, &panel_inner_parser, &nube_config.bottom_panel);
	} else {
		g_set_error(
			err,
			GSDL_SYNTAX_ERROR,
			GSDL_SYNTAX_ERROR_UNEXPECTED_TAG,
			"Unexpected tag: %s",
			name
		);
	}
}

static void _panels_end_tag(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **err
	) {
	if (strcmp(name, "panels") == 0) gsdl_parser_context_pop(context);
}

static GSDLParser panels_parser = {
	_panels_start_tag,
	_panels_end_tag,
	_root_error_handler
};

static void _root_start_tag(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {

	if (strcmp(name, "appearance") == 0) {
		gsdl_parser_context_push(context, &appearance_parser, NULL);
	} else if (strcmp(name, "behavior") == 0) {
		gsdl_parser_context_push(context, &behavior_parser, NULL);
	} else if (strcmp(name, "panels") == 0) {
		gsdl_parser_context_push(context, &panels_parser, NULL);
	} else {
		g_set_error(
			err,
			GSDL_SYNTAX_ERROR,
			GSDL_SYNTAX_ERROR_UNEXPECTED_TAG,
			"Unexpected tag: %s",
			name
		);
	}
}

static GSDLParser root_parser = {
	_root_start_tag,
	NULL,
	_root_error_handler
};

bool nube_config_load() {
	char rc_name[PATH_MAX];
	sprintf(rc_name, "%s/.nube.conf", getenv("HOME"));
	if (!g_file_test(rc_name, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK)) {
		g_printerr("Could not find ~/.nube.conf\n");
		return false;
	}

	parse_err = NULL;
	GSDLParserContext *context = gsdl_parser_context_new(&root_parser, NULL);
	gsdl_parser_context_parse_file(context, rc_name);

	if (parse_err) {
		g_printerr("Could not load ~/.nube.conf: %s\n", parse_err->message);
		return false;
	}

	return true;
}
