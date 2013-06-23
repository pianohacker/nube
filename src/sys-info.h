#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__

double nube_sys_get_memory_usage();
double nube_sys_get_swap_usage();
void nube_sys_get_power(double *energy, double *power);
double nube_sys_get_cpu();

#endif
