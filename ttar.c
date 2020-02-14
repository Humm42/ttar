#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef enum Mode {
	MNone, MCreate, MList, MExtract,
} Mode;

/*
 * compares lhs all-lowercase without whitespace with rhs
 * return 0: equal
 * else: unequal
 */
int
wscmp(char *lhs, char *rhs)
{
	for (; *lhs; ++lhs) {
		if (!isspace(*lhs)) {
			if (tolower(*lhs) != *rhs)
				return 1;
			++rhs;
		}
	}
	return *rhs;
}

void
list_entries(char *filename)
{
	FILE *archive;
	if (strcmp(filename, "stdin") != 0)
		archive = fopen(filename, "r");
	else
		archive = stdin;
	if (!archive)
		die("%s: fopen() failed:", argv0);
	char *line = NULL;
	size_t n = 0;
	char buf[1024];
	size_t buflen;
	errno = 0;
	while (getline(&line, &n, archive) != -1) {
		if (strcmp(line, "---\n") == 0) {
			do {
				if (!fgets(buf, 1024, archive)) {
					if (ferror(archive))
						die("%s: fgets() failed:",
						    argv0);
					else
						die("%s: unexpected EOF in %s",
						    argv0, filename);
				}
			} while ((buflen = strlen(buf)) < 4
			      || strcmp(buf + strlen(buf) - 4, "---\n") != 0);
		}
		char *colon;
		if (!(colon = strchr(line, ':')))
			continue;
		*colon = '\0';
		char *i;
		for (i = colon + 1; isspace(*i); ++i);
		if (wscmp(line, "path") == 0)
			fputs(i, stdout);
	}
	if (errno != 0)
		die("%s: getline() failed:", argv0);
	fclose(archive);
}

void
usage(void)
{
	die("usage: %s [-ctx] [-f archive] [file ...]", argv0);
}

int
main(int argc, char **argv)
{
	Mode mode = MNone;
	char *filename = "stdin";

	ARGBEGIN {
	case 'c':
		if (mode)
			usage();
		else
			mode = MCreate;
		break;
	case 't':
		if (mode)
			usage();
		else
			mode = MList;
		break;
	case 'x':
		if (mode)
			usage();
		else
			mode = MExtract;
		break;
	case 'f':
		filename = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND

	switch (mode) {
	case MCreate:
		die("%s: -c not implemented yet", argv0);
		break;
	case MList:
		list_entries(filename);
		break;
	case MExtract:
		die("%s: -x not implemented yet", argv0);
		break;
	default:
		usage();
	}
}
