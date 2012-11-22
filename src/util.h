#ifndef __UTIL_H__
#define __UTIL_H__

typedef struct {
	double x;
	double y;
} DPoint;

extern char* nube_get_runtime_path(char *argv0);

extern double nube_get_num_file(const char *path);
extern char* nube_get_str_file(const char *path);

#endif
