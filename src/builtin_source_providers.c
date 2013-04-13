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

void _nm_info_init(NubeSource *source, gpointer user_data) {
}

void _nm_info_update(NubeSource *source, gpointer user_data) {
	GValue *value = g_datalist_get_data(&source->data, "value");

	if (value == NULL) {
		g_free(value);
	}

	GData **attributes = (GData**) user_data;

	void
}

void _nm_info_provide_source(const gchar *name, GData *attributes) {
	GData **user_data = g_slice_new0(GData*);

	const gchar *device_name;
	gchar *device_path = NULL;
	nube_datalist_require_value("NetworkManager provided source", attributes, "device", G_TYPE_STRING, &device_name);

	GDBusConnection *con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
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

	GVariant *devices = g_dbus_proxy_call_sync(
		proxy,
		"GetDevices",
		NULL,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		NULL
	);

	for (gsize_t i = 0; i < g_variant_n_children(devices)) {
		GDBusProxy *device_proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM,
			0,
			NULL,
			NM_BUS,
			g_variant_get_string(g_variant_get_child_value(devices, i)),
			NM_BASE_INTERFACE,
			NULL,
			NULL
		);

		if (strcmp(g_variant_get_string(g_dbus_get_cached_property(device_proxy, "Interface"), device_name) == 0) {
			device_path = g_variant_dup_string(g_variant_get_child_value(devices, i));
			break;
		}
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
	nube_source_provider_register("lm-sensors", _lm_sensors_provide_source);
}
