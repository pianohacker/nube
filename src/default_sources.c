// NOT BUILT DIRECTLY; included by sources.c

#include "sys-info.h"

//> Macros
#define _SOURCE_ADD(name, init, update) _NubeSource *name##_source = g_slice_new(_NubeSource); name##_source->init_func = init; name##_source->update_func = update; g_datalist_id_set_data(&available_sources, g_quark_from_static_string(#name), name##_source) 
#define _VALUE_ALLOC(name, type) GValue *name##_value = g_slice_new0(GValue); g_value_init(name##_value, type); g_datalist_id_set_data(&source->data, g_quark_from_static_string(#name), name##_value)
#define _VALUE_SET_DOUBLE(name, value) g_value_set_double(g_datalist_id_get_data(&source->data, g_quark_from_static_string(#name)), value)

void _battery_init(_NubeSource *source) {
	_VALUE_ALLOC(value, G_TYPE_DOUBLE);
	_VALUE_ALLOC(change_over_hour, G_TYPE_DOUBLE);
}

void _battery_update(_NubeSource *source) {
	double energy, power;
	nube_sys_get_power(&energy, &power);
	_VALUE_SET_DOUBLE(value, energy);
	_VALUE_SET_DOUBLE(change_over_hour, power);
}

void _cpu_init(_NubeSource *source) {
	_VALUE_ALLOC(value, G_TYPE_DOUBLE);
}

void _cpu_update(_NubeSource *source) {
	double usage;
	nube_sys_get_cpu(&usage);
	_VALUE_SET_DOUBLE(value, usage);
}

void nube_sources_init() {
	_SOURCE_ADD(battery, _battery_init, _battery_update);
	_SOURCE_ADD(cpu, _cpu_init, _cpu_update);
}
