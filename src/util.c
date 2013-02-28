#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

char* nube_get_runtime_path(char *argv0) {
        char* dir = realpath(dirname(argv0), NULL);

        if (strlen(dir) >= 4 && strcmp(dir + strlen(dir) - 4, "/bin") == 0) {
                dir[strlen(dir) - 4] = 0;
        }

        return dir;
}

double nube_get_num_file(const char *path) {
	int fd = open(path, O_RDONLY);
	char buffer[64];
	buffer[read(fd, buffer, 64)] = '\0';
	close(fd);
	
	return strtod(buffer, NULL);
}

char* nube_get_str_file(const char *path) {
	int fd = open(path, O_RDONLY);
	char *buffer = malloc(64);
	buffer[read(fd, buffer, 64) - 1] = '\0';
	close(fd);
	
	return buffer;
}
