#include <arpa/inet.h>
#include <glib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

static GValue* convert_nm_ipv4(GQuark id, GValue *src) {
	GValue *dest = g_slice_new0(GValue);

	if (G_VALUE_TYPE(src) != G_TYPE_VARIANT || strcmp(g_variant_get_type_string(g_value_get_variant(src)), "aau") != 0) {
		g_value_init(dest, G_VALUE_TYPE(src));
		g_value_copy(src, dest);

		return dest;
	}

	g_value_init(dest, G_TYPE_STRING);

	GVariant *variant = g_value_get_variant(src);
	GVariant *first_config = g_variant_get_child_value(variant, 0);
	guint32 address = g_variant_get_uint32(g_variant_get_child_value(first_config, 0));
	guint32 netmask = g_variant_get_uint32(g_variant_get_child_value(first_config, 1));

	struct in_addr addr = { address };

	g_value_take_string(dest, g_strdup_printf("%s/%d", inet_ntoa(addr), netmask));

	return dest;
}

void nube_builtin_converters_init() {
	nube_converter_register("nm-ipv4", convert_nm_ipv4);
}
