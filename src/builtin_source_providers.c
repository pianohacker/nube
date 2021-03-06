#include <gio/gio.h>
#include <glib.h>
#include <sensors/error.h>
#include <sensors/sensors.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

//> Macros
#define _VALUE_ALLOC(name, type) GValue *name##_value = g_slice_new0(GValue); g_value_init(name##_value, type); g_datalist_id_set_data(&source->data, g_quark_from_static_string(#name), name##_value)
#define _VALUE_SET_INT(name, value) g_value_set_int(g_datalist_id_get_data(&source->data, g_quark_from_static_string(#name)), value)
#define _VALUE_SET_DOUBLE(name, value) g_value_set_double(g_datalist_id_get_data(&source->data, g_quark_from_static_string(#name)), value)

#define NM_BUS "org.freedesktop.NetworkManager"
#define NM_OBJ "/org/freedesktop/NetworkManager"
#define NM_BASE_INTERFACE "org.freedesktop.NetworkManager"

void _command_init(NubeSource *source, gpointer user_data) {
	_VALUE_ALLOC(value, G_TYPE_STRING);
}

void _command_update(NubeSource *source, gpointer user_data) {
	GValue *value = g_datalist_get_data(&source->data, "value");

	int sub_stdout;

	GError *err = NULL;
	g_spawn_async_with_pipes(
		NULL,
		(gchar**) user_data,
		NULL,
		G_SPAWN_SEARCH_PATH,
		NULL,
		NULL,
		NULL,
		NULL,
		&sub_stdout,
		NULL,
		&err
	);

	if (err) {
		g_value_set_string(value, "");
		g_printerr("%s\n", err->message);
		exit(1);
	}

	GString *output = g_string_new(NULL);

	char buf[1024];
	int len = 0;

	while ((len = read(sub_stdout, buf, 1024)) > 0) {
		g_string_append_len(output, buf, len);
	}

	close(sub_stdout);

	g_value_take_string(value, output->str);

	g_string_free(output, FALSE);
}

void _command_provide_source(const gchar *name, GData *attributes) {
	gchar **argv = g_new0(gchar*, 4);
	argv[0] = "sh";
	argv[1] = "-c";

	const gchar *command;
	nube_datalist_require_value("command source", attributes, "content", G_TYPE_STRING, &command);
	argv[2] = (char*) command;

	nube_source_register(name, _command_init, _command_update, argv);
}

void _nm_info_init(NubeSource *source, gpointer user_data) {
}

void _nm_info_update(NubeSource *source, gpointer user_data) {
	GValue *value = g_datalist_get_data(&source->data, "value");

	if (value == NULL) {
		value = g_slice_new0(GValue);
	} else {
		g_value_unset(value);
	}

	GData **attributes = (GData**) user_data;
	gchar *property_name = g_strdup(g_datalist_get_data(attributes, "property"));
	GValue *default_value = g_datalist_get_data(attributes, "default_value");
	char *divider;
	const gchar *object_path = g_datalist_get_data(attributes, "device_path");
	GVariant *result = NULL;
	GDBusProxy *proxy = NULL;

	// String munging to walk a chain of properties, where all but the last are object paths
	while (*property_name) {
		g_debug("Walking property chain: %s", property_name);
		if (*property_name == ':') property_name++;

		if (result != NULL) {
			object_path = g_variant_get_string(result, NULL);
			g_object_unref(proxy);
		}

		gchar *interface, *internal_property;
		interface = property_name;

		divider = strchr(property_name, ':');
		if (divider == NULL) {
			property_name += strlen(property_name);
		} else {
			*divider = '\0';
			property_name = divider + 1;
		}

		internal_property = strrchr(interface, '.');
		*internal_property++ = '\0';

		g_debug("Getting %s from interface %s, object %s", internal_property, interface, object_path);

		proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM,
			0,
			NULL,
			NM_BUS,
			object_path,
			g_strdup_printf("%s.%s", NM_BASE_INTERFACE, interface),
			NULL,
			NULL
		);

		if (!proxy || !(result = g_dbus_proxy_get_cached_property(proxy, internal_property))) {
			g_value_init(value, G_VALUE_TYPE(default_value));
			g_value_copy(default_value, value);
			g_datalist_set_data(&source->data, "value", value);
			return;
		}
	}

	if (strcmp(g_variant_get_type_string(result), "ay") == 0) {
		// Transform an array of bytes to a string
		g_value_init(value, G_TYPE_STRING);
		char contents[g_variant_n_children(result)];
		int i = 0;

		GVariantIter *iter;
		g_variant_get(result, "ay", &iter);

		while (g_variant_iter_loop(iter, "y", contents + i++));

		g_variant_iter_free(iter);

		contents[i] = '\0';

		g_value_set_string(value, contents);
	} else {
		g_dbus_gvariant_to_gvalue(result, value);
	}

	g_debug("Result type is %s", g_type_name(G_VALUE_TYPE(value)));

	g_datalist_set_data(&source->data, "value", value);
}

void _nm_info_provide_source(const gchar *name, GData *attributes) {
	GData **user_data = g_slice_new0(GData*);

	const gchar *device_name, *property_name;
	gchar *device_path = NULL;
	nube_datalist_require_value("NetworkManager provided source", attributes, "device", G_TYPE_STRING, &device_name);
	nube_datalist_require_value("NetworkManager provided source", attributes, "property", G_TYPE_STRING, &property_name);
	GValue *default_value = g_datalist_get_data(&attributes, "default");

	if (default_value) {
		GValue *default_value_copy = g_new0(GValue, 1);
		g_value_init(default_value_copy, G_VALUE_TYPE(default_value));
		g_value_copy(default_value, default_value_copy);
		g_datalist_set_data(user_data, "default_value", default_value_copy);
	}

	g_datalist_set_data(user_data, "property", g_strdup(property_name));

	GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		0,
		NULL,
		NM_BUS,
		NM_OBJ,
		NM_BASE_INTERFACE,
		NULL,
		NULL
	);

	GVariant *devices = g_variant_get_child_value(g_dbus_proxy_call_sync(
		proxy,
		"GetDevices",
		NULL,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		NULL
	), 0);

	g_object_unref(proxy);

	// Find the device with the given name
	for (gsize i = 0; i < g_variant_n_children(devices) && device_path == NULL; i++) {
		GDBusProxy *device_proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM,
			0,
			NULL,
			NM_BUS,
			g_variant_get_string(g_variant_get_child_value(devices, i), NULL),
			NM_BASE_INTERFACE ".Device",
			NULL,
			NULL
		);

		if (strcmp(g_variant_get_string(g_dbus_proxy_get_cached_property(device_proxy, "Interface"), NULL), device_name) == 0) {
			device_path = g_variant_dup_string(g_variant_get_child_value(devices, i), NULL);
		}

		g_object_unref(device_proxy);
	}

	if (device_path == NULL) {
		g_printerr("nm-info: invalid device name: %s\n", device_name);
		exit(1);
	}

	g_datalist_set_data(user_data, "device_path", device_path);

	nube_source_register(name, _nm_info_init, _nm_info_update, user_data);
}

void _lm_sensors_init(NubeSource *source, gpointer user_data) {
	_VALUE_ALLOC(value, G_TYPE_DOUBLE);
}

void _lm_sensors_update(NubeSource *source, gpointer user_data) {
	double val;
	GData **attributes = (GData**) user_data;

	sensors_chip_name *chip = g_datalist_get_data(attributes, "chip");
	sensors_subfeature *subfeature_input = g_datalist_get_data(attributes, "subfeature_input");
	double *max = g_datalist_get_data(attributes, "max");

	g_return_if_fail(chip != NULL);
	g_return_if_fail(subfeature_input != NULL);

	g_return_if_fail(sensors_get_value(chip, subfeature_input->number, &val) == 0);

	_VALUE_SET_DOUBLE(value, val / *max);
}

void _lm_sensors_provide_source(const gchar *name, GData *attributes) {
	GData **user_data = g_slice_new0(GData*);
	int error;
	sensors_chip_name input;

	const gchar *chip_name, *feature_name;
	nube_datalist_require_value("lm-sensors provided source", attributes, "chip", G_TYPE_STRING, &chip_name);
	nube_datalist_require_value("lm-sensors provided source", attributes, "feature", G_TYPE_STRING, &feature_name);

	if ((error = sensors_parse_chip_name(chip_name, &input)) != 0) {
		g_printerr("lm-sensors: invalid chip name: %s\n", sensors_strerror(error));
		exit(1);
	}

	int chip_nr = 0;
	const sensors_chip_name *chip;
	const sensors_feature *feature;

	while ((chip = sensors_get_detected_chips(&input, &chip_nr))) {
		int feature_nr = 0;

		while ((feature = sensors_get_features(chip, &feature_nr)) != NULL) {
			if (strcmp(feature->name, feature_name) == 0) break;

		}

		if (feature != NULL) break;
	}

	if (chip == NULL || feature == NULL) {
		g_printerr("lm-sensors: unknown feature: %s.%s\n", chip_name, feature_name);
		exit(1);
	}

	const sensors_subfeature *subfeature_input, *subfeature_max;
	
	switch (feature->type) {
		case SENSORS_FEATURE_FAN:
			subfeature_input = sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_FAN_INPUT);
			subfeature_max = sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_FAN_MAX);
			break;
		case SENSORS_FEATURE_TEMP:
			subfeature_input = sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_TEMP_INPUT);
			subfeature_max = sensors_get_subfeature(chip, feature, SENSORS_SUBFEATURE_TEMP_CRIT);
			break;
		default:
			g_printerr("lm-sensors: don't know how to handle feature %s.%s of type: %d\n", chip_name, feature_name, feature->type);
			exit(1);
	}

	g_datalist_set_data(user_data, "chip", (gpointer) chip);
	g_datalist_set_data(user_data, "subfeature_input", (gpointer) subfeature_input);

	double *max = g_slice_new0(double);

	if (!nube_datalist_get_value(attributes, "max", G_TYPE_DOUBLE, max)) {
		g_return_if_fail(subfeature_max != NULL);
		g_return_if_fail(sensors_get_value(chip, subfeature_max->number, max) == 0);
	}
	g_datalist_set_data(user_data, "max", max);

	nube_source_register(name, _lm_sensors_init, _lm_sensors_update, user_data);
}

void nube_builtin_source_providers_init() {
	sensors_init(NULL);
	nube_source_provider_register("command", _command_provide_source);
	nube_source_provider_register("lm-sensors", _lm_sensors_provide_source);
	nube_source_provider_register("nm-info", _nm_info_provide_source);
}
