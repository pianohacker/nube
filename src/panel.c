#include <clutter/clutter.h>
#include <glib-object.h>

#include "config.h"
#include "drawing.h"
#include "widgets.h"

ClutterActor* nube_panel_new(gfloat x, gfloat y, gfloat hidden_x, gfloat hidden_y, gfloat width, gfloat height, const NubePanelOptions *options) {
	ClutterActor *panel = clutter_actor_new();
	clutter_actor_set_size(panel, width + GLOW_SIZE * 2, height + GLOW_SIZE * 2);
	clutter_actor_set_position(panel, hidden_x, hidden_y);

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
	g_signal_connect(content, "draw", G_CALLBACK(nube_draw_cut_rect), (NubeWidgetOptions *) options);
	clutter_canvas_set_size(CLUTTER_CANVAS(content), width + GLOW_SIZE * 2, height + GLOW_SIZE * 2);
	clutter_actor_set_content(panel, content);

	return panel;
}
