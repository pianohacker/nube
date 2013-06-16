#include <arpa/inet.h>
#include <glib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

static void convert_nm_ipv4(GQuark id, GValue *value) {
	if (G_VALUE_TYPE(value) != G_TYPE_VARIANT || strcmp(g_variant_get_type_string(g_value_get_variant(value)), "aau") != 0) {
		g_debug( "Unexpected type of %p: %s", value, (G_VALUE_TYPE(value) == G_TYPE_VARIANT ? g_variant_get_type_string(g_value_get_variant(value)) : g_type_name(G_VALUE_TYPE(value))));
		return;
	}

	GVariant *variant = g_value_get_variant(value);
	GVariant *first_config = g_variant_get_child_value(variant, 0);
	guint32 address = g_variant_get_uint32(g_variant_get_child_value(first_config, 0));
	guint32 netmask = g_variant_get_uint32(g_variant_get_child_value(first_config, 1));

	struct in_addr addr = { address };

	g_value_unset(value);
	g_value_init(value, G_TYPE_STRING);
	g_value_take_string(value, g_strdup_printf("%s/%d", inet_ntoa(addr), netmask));
}

void nube_builtin_converters_init() {
	nube_converter_register("nm-ipv4", convert_nm_ipv4);
}
