#include <cairo.h>
#include <clutter/clutter.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "drawing.h"
#include "util.h"

static cairo_pattern_t* nube_offset_quads(cairo_path_t *path, double offset) {
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

void nube_draw_panel_poly(ClutterActor *actor, cairo_t *cr, NubePanelConfig *panel_config) {
	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_restore(cr);

	cairo_translate(cr, GLOW_SIZE, GLOW_SIZE);

	for (int i = 0; i < panel_config->shape_elems->length; i += 2) {
		cairo_line_to(
			g_array_index(panel_config->shape_elems, double, i),
			g_array_index(panel_config->shape_elems, double, i+1)
		);
	}

	cairo_close_path(cr);
	cairo_set_source(cr, nube_offset_quads(cairo_copy_path(cr), nube_config.glow_size);
	cairo_paint(cr);

	clutter_cairo_set_source_color(cr, panel_config->background);
	cairo_fill_preserve(cr);
}
