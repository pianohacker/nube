#include <clutter/clutter.h>
#include <glib-object.h>

#include "config.h"
#include "drawing.h"
#include "widgets.h"

ClutterActor* nube_panel_new(gfloat x, gfloat y, gfloat hidden_x_shift, gfloat hidden_y_shift, const NubePanelConfig *panel_config) {
	gfloat width = panel_config->shape_max_x;
	gfloat height = panel_config->shape_max_y;
	ClutterActor *panel = clutter_actor_new();
	clutter_actor_set_size(panel, width + nube_config.glow_size * 2, height + nube_config.glow_size * 2);
	clutter_actor_set_position(
		panel,
		x + hidden_x_shift * (width + nube_config.glow_size),
		y + hidden_y_shift * (height + nube_config.glow_size)
	);

	/*for (int i = 0; i < MAX_WIDGETS_SIZE && options->widget_opts[i].type; i++) {*/
		/*nube_add_widget(layout, &options->widget_opts[i]);*/
	/*}*/

	ClutterPoint *shown_point = clutter_point_alloc();
	clutter_point_init(shown_point,
		x + hidden_x_shift * nube_config.glow_size,
		y + hidden_y_shift * nube_config.glow_size
	);
	g_object_set_data(G_OBJECT(panel), "shown_point", shown_point);

	ClutterPoint *hidden_point = clutter_point_alloc();
	clutter_actor_get_position(panel, &hidden_point->x, &hidden_point->y);
	g_object_set_data(G_OBJECT(panel), "hidden_point", hidden_point);

	ClutterContent *content = clutter_canvas_new();
	g_signal_connect(content, "draw", G_CALLBACK(nube_draw_panel_poly), (NubePanelConfig *) panel_config);
	clutter_canvas_set_size(CLUTTER_CANVAS(content), width + nube_config.glow_size * 2, height + nube_config.glow_size * 2);
	clutter_actor_set_content(panel, content);

	return panel;
}
