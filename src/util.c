#include <fcntl.h>
#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

char* nube_get_runtime_path(const char *argv0) {
	char* dir = realpath(dirname(strdup(argv0)), NULL);

	if (strlen(dir) >= 4 && strcmp(dir + strlen(dir) - 4, "/bin") == 0) {
			dir[strlen(dir) - 4] = 0;
	}

	return dir;
}

double nube_get_num_file(const char *path) {
	int fd = open(path, O_RDONLY);
	char buffer[64];
	buffer[read(fd, buffer, 64)] = '\0';
	close(fd);
	
	return strtod(buffer, NULL);
}

char* nube_get_str_file(const char *path) {
	int fd = open(path, O_RDONLY);
	char *buffer = malloc(64);
	buffer[read(fd, buffer, 64) - 1] = '\0';
	close(fd);
	
	return buffer;
}

bool nube_datalist_id_get_value_v(GData *datalist, GQuark key_id, GType type, va_list args) {
	GValue *value = g_datalist_id_get_data(&datalist, key_id);
	// This just means that the value in question doesn't exist, which
	// is fine and shouldn't generate a critical log
	if (value == NULL) return false;

	if (type && G_VALUE_TYPE(value) != type) {
		if (g_value_type_transformable(G_VALUE_TYPE(value), type)) {
			GValue *new_value = g_slice_new0(GValue);
			g_value_init(new_value, type);
			g_value_transform(value, new_value);
			value = new_value;
		} else {
			g_printerr("Attribute %s should be %s, got %s", g_quark_to_string(key_id), g_type_name(type), g_type_name(G_VALUE_TYPE(value)));
		}
	}

	gchar *err = NULL;
	G_VALUE_LCOPY(value, args, 0, &err);
	g_return_val_if_fail(err == NULL, false);

	return true;
}

bool nube_datalist_id_get_value(GData *datalist, GQuark key_id, GType type, ...) {
	va_list args;
	va_start(args, type);

	bool result = nube_datalist_id_get_value_v(datalist, key_id, type, args);

	va_end(args);

	return result;
}

void nube_datalist_require_value(const gchar *container_name, GData *datalist, const gchar *key, GType type, ...) {
	va_list args;
	va_start(args, type);

	bool success = nube_datalist_id_get_value_v(datalist, g_quark_from_string(key), type, args);
	va_end(args);

	if (!success) {
		g_printerr("Attribute %s required in %s\n", key, container_name);
		exit(1);
	}
}
