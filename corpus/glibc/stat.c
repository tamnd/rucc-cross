/* stat, fstat and lstat, which are the reason validation 3 exists.
 *
 * glibc moved these twice. Until 2.33 the header turned a call to `stat` into a call to `__xstat`
 * with a leading `_STAT_VER` argument, because the shape of `struct stat` had changed once in the
 * 1990s and the library wanted to know which shape the caller was compiled against. From 2.33 on
 * there is a real `stat` symbol and the header calls it directly. Nothing about that is visible in
 * a struct layout test or in a macro dump: both spellings define the same `struct stat` and the
 * same `_STAT_VER`, and the difference is which symbol the object ends up referencing.
 *
 * So the only way to be wrong here is to compile against one release's header and run against
 * another release's library, and the only way to find out is to run. A binary built with the 2.28
 * spelling calls `__xstat`, which 2.39 still exports as a compatibility symbol, and a binary built
 * with the 2.39 spelling calls `stat`, which 2.28 does not export at all and which fails to load
 * rather than failing at the call. That second direction is the one this catches.
 *
 * The checks are on values the kernel decides rather than on sizes the header decides, because a
 * header that gets the layout wrong reads the wrong offsets out of a right answer. The file is
 * written here rather than looked for, so the size and the mode are known without asking anything
 * else about the machine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static const char text[] = "0123456789";

int main(void)
{
	struct stat by_path;
	struct stat by_fd;
	struct stat by_link;
	char name[] = "/tmp/rucc-stat-XXXXXX";
	int fd = mkstemp(name);

	if (fd < 0) {
		printf("mkstemp failed\n");
		return 1;
	}
	if (write(fd, text, sizeof(text) - 1) != (ssize_t)(sizeof(text) - 1)) {
		printf("write failed\n");
		return 1;
	}

	if (fstat(fd, &by_fd) != 0 || stat(name, &by_path) != 0 || lstat(name, &by_link) != 0) {
		printf("a stat call failed\n");
		return 1;
	}
	close(fd);
	unlink(name);

	/* The size is the one field a wrong offset cannot accidentally agree on, since it is the only
	 * field here whose value this program chose. */
	printf("size %d\n", (int)by_path.st_size);
	printf("same %d\n", by_fd.st_size == by_path.st_size && by_fd.st_ino == by_path.st_ino);

	/* S_ISREG reads the mode through the macros, so a mode at the wrong offset says not a file. */
	printf("regular %d\n", S_ISREG(by_path.st_mode) != 0);
	printf("link %d\n", S_ISLNK(by_link.st_mode) == 0 && S_ISREG(by_link.st_mode) != 0);

	/* st_nlink and st_mode are small fields next to large ones, so they are where a layout that is
	 * right about st_size can still be wrong. A temporary file has exactly one link. */
	printf("links %d\n", (int)by_path.st_nlink);

	/* The timestamps are the tail of the structure and the part 2.34 rearranged for 64-bit time on
	 * 32-bit machines. A file written a moment ago has a modification time that is not zero and is
	 * not in the far future, which is all that can be said without a clock to compare against. */
	printf("mtime %d\n", by_path.st_mtime > 1000000000 && by_path.st_mtime < 4000000000u);

	return 0;
}
