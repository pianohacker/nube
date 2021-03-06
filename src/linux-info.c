#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

double nube_sys_get_memory_usage() {
	FILE *meminfo = fopen("/proc/meminfo", "r");

	if (!meminfo) return 0;

	char *line = NULL;
	size_t size;
	unsigned long mem_total, mem_free, mem_buffers, mem_cached;
	int elems_found = 0;

	while (!feof(meminfo) && elems_found < 4) {
		getline(&line, &size, meminfo);

		elems_found += (
			sscanf(line, "MemTotal: %lu", &mem_total) +
			sscanf(line, "MemFree: %lu", &mem_free) +
			sscanf(line, "Buffers: %lu", &mem_buffers) +
			sscanf(line, "Cached: %lu", &mem_cached)
		);
	}

	fclose(meminfo);

	return (mem_total - mem_free - mem_buffers - mem_cached) / (double) mem_total;
}

double nube_sys_get_swap_usage() {
	FILE *meminfo = fopen("/proc/meminfo", "r");

	if (!meminfo) return 0;

	char *line = NULL;
	size_t size;
	unsigned long swap_total, swap_free;
	int elems_found = 0;

	while (!feof(meminfo) && elems_found < 2) {
		getline(&line, &size, meminfo);

		elems_found += (
			sscanf(line, "SwapTotal: %lu", &swap_total) +
			sscanf(line, "SwapFree: %lu", &swap_free)
		);
	}

	fclose(meminfo);

	return swap_total == 0 ? 0 : ((swap_total - swap_free) / (double) swap_total);
}

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

	fclose(stat);
	return result;
}
