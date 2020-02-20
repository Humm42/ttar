#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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

void	add_file_to_archive(FILE *, char *);
void	add_regfile_to_archive(FILE *, char *, struct stat *);
void	add_dir_to_archive(FILE *, char *, struct stat *);

void
add_regfile_to_archive(FILE *archive, char *filename, struct stat *stats)
{
	fputs("type:\tregular file\n", archive);
	FILE *file;
	if (!(file = fopen(filename, "rb")))
		die("%s: fopen() on ‘%s’ failed:", argv0, filename);
	fprintf(archive, "file size:\t%ld\n", stats->st_size);
	fputs("---\n", archive);

	char buf[8192];
	size_t readbytes;
	while ((readbytes = fread(buf, 1, 8192, file)) > 0) {
		errno = 0;
		fwrite(buf, 1, readbytes, archive);
		if (errno)
			die("%s: fwrite() to archive failed:", argv0);
	}
	if (!feof(file))
		die("%s: fread() from ‘%s’ failed:", argv0, filename);
	fputs("---\n", archive);

	fclose(file);
}

void
add_dir_to_archive(FILE *archive, char *filename, struct stat *stats)
{
	UNUSED(stats);
	fputs("type:\tdirectory\n", archive);

	DIR *dir;
	if (!(dir = opendir(filename)))
		die("%s: opendir() ‘%s’ failed:", argv0, filename);
	struct dirent *de;
	errno = 0;
	while ((de = readdir(dir))) {
		if (strcmp(de->d_name, ".") != 0
		 && strcmp(de->d_name, "..") != 0) {
			size_t lenold = strlen(filename),
			       lennew = strlen(de->d_name);
			char *newname = emalloc(lenold + lennew + 2);
			strcpy(newname, filename);
			newname[lenold] = '/';
			strcpy(newname + lenold + 1, de->d_name);
			add_file_to_archive(archive, newname);
			free(newname);
		}
	}
	if (errno != 0)
		die("%s: readdir() from ‘%s’ failed:",
		    argv0, filename);
	if (closedir(dir) < 0)
		die("%s: closedir() on ‘%s’ failed:", argv0, filename);
}

void
add_file_to_archive(FILE *archive, char *filename)
{
	struct stat stats;
	if (lstat(filename, &stats) < 0)
		die("%s: lstat() on ‘%s’ failed:", argv0, filename);

	fputc('\n', archive);
	fprintf(archive, "path:\t%s\n", filename);
	fprintf(archive, "permissions:\t%.4o\n",
	        stats.st_mode & 07777);
	fprintf(archive, "user id:\t%d\n", stats.st_uid);
	fprintf(archive, "group id:\t%d\n", stats.st_gid);
	struct passwd *pws = getpwuid(stats.st_uid);
	fprintf(archive, "user name:\t%s\n", pws->pw_name);
	struct group *grp = getgrgid(stats.st_gid);
	fprintf(archive, "group name:\t%s\n", grp->gr_name);
	fprintf(archive, "modification time:\t%ld\n",
	        stats.st_mtim.tv_sec);

	if (S_ISREG(stats.st_mode)) {
		add_regfile_to_archive(archive, filename, &stats);
	} else if (S_ISBLK(stats.st_mode)) {
		fputs("type:\tblock device\n", archive);
	} else if (S_ISCHR(stats.st_mode)) {
		fputs("type:\tcharacter device\n", archive);
	} else if (S_ISDIR(stats.st_mode)) {
		add_dir_to_archive(archive, filename, &stats);
	} else if (S_ISFIFO(stats.st_mode)) {
		fputs("type:\tfifo\n", archive);
	} else if (S_ISLNK(stats.st_mode)) {
		fputs("type:\tsymbolic link\n", archive);
		char *linktarg = emalloc(stats.st_size+1);
		readlink(filename, linktarg, stats.st_size);
		linktarg[stats.st_size] = '\0';
		fprintf(archive, "link target:\t%s\n", linktarg);
		free(linktarg);
	} else if (S_ISSOCK(stats.st_mode)) {
		fputs("type:\tsocket\n", archive);
	} else
		die("%s: ‘%s’ has unsupported type", argv0, filename);
}

void
create_archive(char *filename, char **files)
{
	FILE *archive;
	if (strcmp(filename, "stdin"))
		archive = fopen(filename, "w");
	else
		archive = stdout;
	if (!archive)
		die("%s: fopen() ‘%s’ failed:", argv0, filename);

	fputs("metadata encoding:\tutf8\n", archive);

	time_t seconds;
	time(&seconds);
	if (seconds == (time_t)-1)
		die("%s: time() failed:", argv0);
	struct tm *brtime = gmtime(&seconds);
	if (!brtime)
		die("%s: gmtime() failed:", argv0);
	char timestring[21];
	errno = 0;
	if (strftime(timestring, 21, "%Y-%m-%dT%H:%M:%SZ", brtime) == 0) {
		if (errno)
			die("%s: strftime() failed:", argv0);
		else
			die("%s: strftime() failed", argv0);
	}
	fprintf(archive, "archive creation date:\t%s\n", timestring);

	for (; *files; ++files) {
		add_file_to_archive(archive, files[0]);
	}

	fclose(archive);
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
		die("%s: fopen() ‘%s’ failed:", argv0, filename);
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
						die("%s: fgets() from archive "
						    "failed:", argv0);
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
		die("%s: getline() from archive failed:", argv0);
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
		create_archive(filename, argv);
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
