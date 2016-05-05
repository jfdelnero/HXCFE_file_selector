#include <stdio.h>
#include <stdlib.h>
#include <proto/dos.h>
#include <errno.h>
int truncate(const char *path, off_t length) {
	int retval;
	BPTR fd = Open(path, MODE_OLDFILE);
	if (fd != NULL) {
		retval = (SetFileSize(fd, length, OFFSET_BEGINNING) >= 0) ? 0 : -1;
		Close(fd);
	} else {
	   errno = ENOENT;
	   retval = -1;
	}

	return retval;
}
