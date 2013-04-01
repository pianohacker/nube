// NOT BUILT DIRECTLY; included by sources.c

#include "sys-info.h"

//> Macros
#define _VALUE_ALLOC(name, type) GValue *name##_value = g_slice_new0(GValue); g_value_init(name##_value, type); g_datalist_id_set_data(&source->data, g_quark_from_static_string(#name), name##_value)
#define _VALUE_SET_INT(name, value) g_value_set_int(g_datalist_id_get_data(&source->data, g_quark_from_static_string(#name)), value)
#define _VALUE_SET_DOUBLE(name, value) g_value_set_double(g_datalist_id_get_data(&source->data, g_quark_from_static_string(#name)), value)

void _battery_init(NubeSource *source, gpointer user_data) {
	_VALUE_ALLOC(value, G_TYPE_DOUBLE);
	_VALUE_ALLOC(change_over_hour, G_TYPE_DOUBLE);
}

void _battery_update(NubeSource *source, gpointer user_data) {
	double energy, power;
	nube_sys_get_power(&energy, &power);
	_VALUE_SET_DOUBLE(value, energy);
	_VALUE_SET_DOUBLE(change_over_hour, power);
}

void _cpu_init(NubeSource *source, gpointer user_data) {
	_VALUE_ALLOC(value, G_TYPE_DOUBLE);
}

void _cpu_update(NubeSource *source, gpointer user_data) {
	double usage = nube_sys_get_cpu();
	_VALUE_SET_DOUBLE(value, usage);
}

void _memory_usage_init(NubeSource *source, gpointer user_data) {
	_VALUE_ALLOC(value, G_TYPE_DOUBLE);
}

void _memory_usage_update(NubeSource *source, gpointer user_data) {
	double usage = nube_sys_get_memory_usage();
	_VALUE_SET_DOUBLE(value, usage);
}

void nube_builtin_sources_init() {
	nube_source_register("battery", _battery_init, _battery_update, NULL);
	nube_source_register("cpu", _cpu_init, _cpu_update, NULL);
	nube_source_register("memory_usage", _memory_usage_init, _memory_usage_update, NULL);
}
