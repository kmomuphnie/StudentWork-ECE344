#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "wc.h"

int
main(int argc, char *argv[])
{
	int len = 0;
	int fd;
	char c;
	off_t i;
	void *addr;
	struct stat sb;
	char word[1024];
	struct wc *wc;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(1);
	}
	/* open file */
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "open: %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	/* obtain file size */
	if (fstat(fd, &sb) < 0) {
		fprintf(stderr, "fstat: %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	/* map the whole file */
	addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		fprintf(stderr, "mmap: %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	wc = wc_init();
	for (i = 0; i < sb.st_size; i++) {
		/* whitespace finishes a word. */
		c = ((char *)addr)[i];
		if (isspace(c)) {
			if (len > 0) {
				word[len] = 0;
				wc_insert_word(wc, word);
				len = 0;
			}
		} else {
			word[len] = c;
			len++;
		}
	}
	if (len > 0) {
		wc_insert_word(wc, word);
	}
	wc_output(wc);
	exit(0);
}
