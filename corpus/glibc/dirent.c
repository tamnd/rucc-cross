/* readdir, which is a structure the kernel fills and the header describes.
 *
 * struct dirent is the one place where a header being wrong produces a plausible answer rather than
 * a crash. d_name is a trailing array and everything before it is a size, an offset and two small
 * fields, so a header that puts d_name one word early reads file names that are shifted and a
 * header that gets d_type wrong calls every entry unknown. Both look like a filesystem problem.
 *
 * The large file dimension is the other half. readdir and readdir64 are different symbols with
 * different structures, and which one a call gets is decided by _FILE_OFFSET_BITS, which is decided
 * by the header. bin/glibc-image-check compiles everything here twice for that reason, once plain
 * and once with _FILE_OFFSET_BITS 64, and both have to give the same answer.
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	char root[] = "/tmp/rucc-dirent-XXXXXX";
	char path[128];
	DIR *dir;
	struct dirent *entry;
	int seen = 0;
	int named = 0;
	int typed = 0;
	int dots = 0;
	int i;

	if (mkdtemp(root) == NULL) {
		printf("mkdtemp failed\n");
		return 1;
	}

	/* Three files whose names are long enough that a d_name read from the wrong offset loses its
	 * first characters rather than finding a zero and calling itself empty. */
	for (i = 0; i < 3; i++) {
		FILE *file;
		snprintf(path, sizeof path, "%s/entry-number-%d", root, i);
		file = fopen(path, "w");
		if (file == NULL) {
			printf("fopen failed\n");
			return 1;
		}
		fclose(file);
	}

	dir = opendir(root);
	if (dir == NULL) {
		printf("opendir failed\n");
		return 1;
	}
	while ((entry = readdir(dir)) != NULL) {
		seen++;
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			dots++;
			continue;
		}
		if (strncmp(entry->d_name, "entry-number-", 13) == 0 && strlen(entry->d_name) == 14)
			named++;
		if (entry->d_type == DT_REG)
			typed++;
	}
	closedir(dir);

	for (i = 0; i < 3; i++) {
		snprintf(path, sizeof path, "%s/entry-number-%d", root, i);
		unlink(path);
	}
	rmdir(root);

	printf("seen %d\n", seen);
	printf("dots %d\n", dots);
	printf("named %d\n", named);
	printf("typed %d\n", typed);
	return 0;
}
