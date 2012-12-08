#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

void nube_sys_get_power(double *energy, double *power) {
	char *status = nube_get_str_file(BATTERY_PREFIX "/status");
	double energy_full = nube_get_num_file(BATTERY_PREFIX "/energy_full");
	double energy_now = nube_get_num_file(BATTERY_PREFIX "/energy_now");
	double power_now = nube_get_num_file(BATTERY_PREFIX "/power_now");

	*energy = energy_now / energy_full;

	if (strcmp(status, "Discharging") == 0) {
		*power = -power_now / energy_full;
	} else if (strcmp(status, "Charging") == 0) {
		*power = power_now / energy_full;
	} else {
		*power = 0;
	}

	free(status);
}


double nube_sys_get_cpu() {
	static unsigned long last_total = 0;
	static unsigned long last_active_total = 0;

	FILE *stat = fopen("/proc/stat", "r");

	if (!stat) return 0;

	char *line = NULL;
	size_t size;
	unsigned long time_user, time_nice, time_system, time_idle, time_iowait, time_irq, time_softirq, time_steal, time_guest, time_guest_nice;
	bool success = false;

	while (!feof(stat)) {
		getline(&line, &size, stat);

		if (
				strncmp(line, "cpu ", 4) == 0 &&
				sscanf(line, "cpu  %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &time_user, &time_nice, &time_system, &time_idle, &time_iowait, &time_irq, &time_softirq, &time_steal, &time_guest, &time_guest_nice) == 10
			) {
			success = true;
			break;
		}
	}

	if (!success) return 0;

	unsigned long total = time_user + time_nice + time_system + time_idle + time_iowait + time_irq + time_softirq + time_steal + time_guest + time_guest_nice;
	unsigned long active_total = total - time_idle - time_iowait;
	double result = 0;

	if (last_total && last_active_total) {
		result = ((double) (active_total - last_active_total)) / (total - last_total);
	}

	last_total = total;
	last_active_total = active_total;

	return result;
}
