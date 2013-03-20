#include <cairo.h>
#include <clutter/clutter.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "drawing.h"
#include "util.h"

static DPoint _line_intersect(
		double x1, double y1,
		double x2, double y2,
		double x3, double y3,
		double x4, double y4
	) {
	return (DPoint) {
		((x1*y2-y1*x2)*(x3-x4)-(x1-x2)*(x3*y4-y3*x4)) / ((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4)),
		((x1*y2-y1*x2)*(y3-y4)-(y1-y2)*(x3*y4-y3*x4)) / ((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4))
	};
}

static cairo_pattern_t* nube_offset_quads(cairo_path_t *path, double offset) {
	cairo_pattern_t *result = cairo_pattern_create_mesh();
	cairo_path_data_t *cur_data = path->data;
	cairo_path_data_t *end = path->data + path->num_data;

	int path_len = path->num_data / 2;
	double norm_angles[path_len];
	//double *angle_spans = malloc(sizeof(double) * path_len);
	DPoint points[path_len];
	DPoint line_starts[path_len];
	DPoint line_ends[path_len];
	DPoint intersect_points[path_len];

	for (; cur_data < end; cur_data++) {
		if (cur_data->header.type != CAIRO_PATH_MOVE_TO) continue;

		int num_points = 1;
		points[0] = (DPoint) {cur_data[1].point.x, cur_data[1].point.y};
		
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
			for (int i = 0; i < path_len; i++) intersect_points[i].x = NAN;

			i++;
		}
		cur_data = path_data;

		for (i = 0; i < num_points; i++) {
			int next_i = (i + 1) % num_points;

			line_starts[i] = (DPoint) {
				points[i].x + cos(norm_angles[i]) * nube_config.glow_size,
				points[i].y + sin(norm_angles[i]) * nube_config.glow_size
			};

			line_ends[i] = (DPoint) {
				points[next_i].x + cos(norm_angles[i]) * nube_config.glow_size,
				points[next_i].y + sin(norm_angles[i]) * nube_config.glow_size
			};
		}

		for (i = 0; i < num_points; i++) {
			int last_i = (i - 1 + num_points) % num_points;
			int next_i = (i + 1) % num_points;

			if (isnan(intersect_points[i].x)) {
				intersect_points[i] = _line_intersect(
					line_starts[last_i].x,
					line_starts[last_i].y,
					line_ends[last_i].x,
					line_ends[last_i].y,
					line_starts[i].x,
					line_starts[i].y,
					line_ends[i].x,
					line_ends[i].y
				);
			}

			if (isnan(intersect_points[next_i].x)) {
				intersect_points[next_i] = _line_intersect(
					line_starts[i].x,
					line_starts[i].y,
					line_ends[i].x,
					line_ends[i].y,
					line_starts[next_i].x,
					line_starts[next_i].y,
					line_ends[next_i].x,
					line_ends[next_i].y
				);
			}

			cairo_mesh_pattern_begin_patch(result);
			cairo_mesh_pattern_move_to(result, points[next_i].x, points[next_i].y);
			cairo_mesh_pattern_line_to(result, points[i].x, points[i].y);
			cairo_mesh_pattern_line_to(result, intersect_points[i].x, intersect_points[i].y);
			cairo_mesh_pattern_line_to(result, intersect_points[next_i].x, intersect_points[next_i].y);

			cairo_mesh_pattern_set_corner_color_rgba(result, 0, 1, 1, 1, 0.4);
			cairo_mesh_pattern_set_corner_color_rgba(result, 1, 1, 1, 1, 0.4);
			cairo_mesh_pattern_set_corner_color_rgba(result, 2, 1, 1, 1, 0);
			cairo_mesh_pattern_set_corner_color_rgba(result, 3, 1, 1, 1, 0);
			cairo_mesh_pattern_end_patch(result);
		}
	}

	return result;
}

void nube_draw_panel_poly(ClutterActor *actor, cairo_t *cr, gint width, gint height, NubePanelConfig *panel_config) {
	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_restore(cr);

	cairo_translate(cr, nube_config.glow_size, nube_config.glow_size);

	for (int i = 0; i < panel_config->shape_elems->len; i += 2) {
		cairo_line_to(
			cr,
			g_array_index(panel_config->shape_elems, double, i),
			g_array_index(panel_config->shape_elems, double, i+1)
		);
	}

	cairo_close_path(cr);
	cairo_set_source(cr, nube_offset_quads(cairo_copy_path(cr), nube_config.glow_size));
	cairo_paint(cr);

	clutter_cairo_set_source_color(cr, panel_config->background);
	cairo_fill_preserve(cr);
}
