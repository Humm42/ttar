#include <stdlib.h>

#include "util.h"

typedef enum Mode {
	MNone, MCreate, MList, MExtract,
} Mode;

void
usage(void)
{
	die("usage: %s [-ctx] [-f archive] [file ...]", argv0);
}

int
main(int argc, char **argv)
{
	Mode mode = MNone;
	char *filename = "/dev/stdin";

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
		die("%s: -t not implemented yet", argv0);
		break;
	case MExtract:
		die("%s: -x not implemented yet", argv0);
		break;
	default:
		usage();
	}
}
