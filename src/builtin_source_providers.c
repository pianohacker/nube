NubeSource* _lm_sensors_provide_source(NubeSource *source, gchar* const *attr_names, GValue* const *attr_values) {
}

void nube_builtin_source_providers_init() {
	nube_source_provider_register("lm-sensors", _lm_sensors_provide_source);
}
