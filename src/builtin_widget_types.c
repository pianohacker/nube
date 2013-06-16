// NOT BUILT DIRECTLY; included by widgets.c

static ClutterActor* _canvas_widget_new(GCallback draw_cb, NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_actor_new();

	ClutterContent *content = clutter_canvas_new();
	g_signal_connect(content, "draw", draw_cb, (NubeWidgetConfig*) widget_config);
	g_signal_connect(widget, "notify::allocation", G_CALLBACK(_canvas_resize_cb), content);
	clutter_actor_set_content(widget, content);

	return widget;
}

static void _clock_draw(ClutterActor *widget, NubeWidgetConfig *widget_config) {
	char buffer[256];
	time_t local;
	const gchar *format;
	nube_datalist_require_value("clock widget", widget_config->props, "format", G_TYPE_STRING, &format);

	time(&local);
	strftime(buffer, 64, format, localtime(&local));
	clutter_text_set_text(CLUTTER_TEXT(widget), buffer);
}

static ClutterActor* _text_init(NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_text_new();
	clutter_text_set_single_line_mode(CLUTTER_TEXT(widget), TRUE);
	clutter_text_set_color(CLUTTER_TEXT(widget), nube_config.fg);

	const gchar *align;

	if (nube_datalist_get_value(widget_config->props, "align", G_TYPE_STRING, &align) && strcmp(align, "right") == 0) {
		// Hack because anchor-from-gravity is deprecated for some retarded reason
		g_signal_connect(widget, "notify::allocation", G_CALLBACK(_align_right_resize_cb), NULL);
	}

	return widget;
}

static void _text_draw(ClutterActor *widget, NubeWidgetConfig *widget_config) {
	gchar *value;
	nube_source_get(widget_config->source_id, "value", G_TYPE_STRING, &value);
	clutter_text_set_text(CLUTTER_TEXT(widget), value);
}

void _vertical_bar_render(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubeWidgetConfig *widget_config) {
	double value, change_over_hour = 0;
	nube_source_get(widget_config->source_id, "value", G_TYPE_DOUBLE, &value);
	nube_source_get(widget_config->source_id, "change_over_hour", G_TYPE_DOUBLE, &change_over_hour);
	double value_left = 1 - value;
	ClutterColor partial_color = WIDGET_PARTIAL_COLOR;

	_cairo_clear(cr);
	
	// The math munging below is to handle the narrowing of the amount that will increase/decrease
	// over the next hour, including when that amount is greater than that remaining
	if (change_over_hour < 0) {
		clutter_cairo_set_source_color(cr, &partial_color);

		cairo_rectangle(cr,
			0, 0,
			width, height * value_left
		);
		cairo_fill(cr);

		clutter_cairo_set_source_color(cr, nube_config.fg);

		if (value < -change_over_hour) {
			cairo_rectangle(cr,
				width * .3, height,
				width * .4, height * value / change_over_hour * value
			);
			cairo_fill(cr);
		} else {
			cairo_rectangle(cr,
				width * .3, height * value_left,
				width * .4, height * -change_over_hour
			);
			cairo_fill(cr);

			cairo_rectangle(cr,
				0, height * (value_left - change_over_hour),
				width, height * value + change_over_hour
			);
			cairo_fill(cr);
		}
	} else {
		clutter_cairo_set_source_color(cr, &partial_color);

		if (value_left < change_over_hour) {
			cairo_rectangle(cr,
				width * .3, 0,
				width * .4, height * value_left / change_over_hour * value_left
			);
			cairo_fill(cr);
		} else {
			cairo_rectangle(cr,
				0, 0,
				width, height * (value_left - change_over_hour)
			);
			cairo_fill(cr);

			cairo_rectangle(cr,
				width * .3, height * (value_left - change_over_hour),
				width * .4, height * change_over_hour
			);
			cairo_fill(cr);
		}

		clutter_cairo_set_source_color(cr, nube_config.fg);

		cairo_rectangle(cr,
			0, height * value_left,
			width, height * value
		);
		cairo_fill(cr);
	}
}

static ClutterActor* _vertical_bar_init(NubeWidgetConfig *widget_config) {
	ClutterActor *widget = _canvas_widget_new(G_CALLBACK(_vertical_bar_render), widget_config);

	return widget;
}

static void _vertical_bar_draw(ClutterActor *widget, NubeWidgetConfig *widget_config) {
	clutter_content_invalidate(clutter_actor_get_content(widget));
}

static ClutterActor* _icon_init(NubeWidgetConfig *widget_config) {
	ClutterActor *widget = clutter_actor_new();
	char image_filename[PATH_MAX];
	const gchar *image_base;
	nube_datalist_require_value("icon widget", widget_config->props, "file", G_TYPE_STRING, &image_base);

	sprintf(image_filename, "%s/nube/icons/%s", g_get_user_config_dir(), image_base);

	GError *err = NULL;
	ClutterContent *content = clutter_image_new();
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_filename, &err);

	if (err) {
		g_printerr("%s\n", err->message);
		exit(1);
	}

	if (!err) {
		// Yes, this really is the way to load images...
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

void nube_builtin_widget_types_init() {
	nube_widget_type_register("clock", _text_init, _clock_draw);
	nube_widget_type_register("icon", _icon_init, NULL);
	nube_widget_type_register("text", _text_init, _text_draw);
	nube_widget_type_register("vertical_bar", _vertical_bar_init, _vertical_bar_draw);
}
