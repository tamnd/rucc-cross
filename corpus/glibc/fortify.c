/* _FORTIFY_SOURCE, which is the header calling functions that only some releases export.
 *
 * With fortification on, the header rewrites memcpy into __memcpy_chk, printf into __printf_chk and
 * so on, and each of those is a symbol in the library rather than something the compiler provides.
 * Which ones exist depends on the release: __printf_chk is old, __snprintf_chk is old, and the
 * fortified wide character and read-like functions arrived over several releases. glibc 2.34 added
 * level 3, which rewrites more calls than level 2 and needs __builtin_dynamic_object_size.
 *
 * That makes this the one program here whose failure mode is at load time rather than at run time.
 * A merged tree that hands a 2.28 target the 2.39 spelling of a fortified call produces a binary
 * that names a symbol 2.28 does not have, and the loader says so and exits. Which is a good failure,
 * and the point of running it on a real image is that nothing before this step would have noticed.
 *
 * bin/glibc-image-check compiles this at -O2, because the header only fortifies when the compiler
 * is optimizing, and at both fortification levels the tree's oldest release supports.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	char small[16];
	char large[64];
	int written;

	memset(large, 'x', sizeof large);
	large[sizeof large - 1] = '\0';

	/* Each of these is a different rewrite in the header and a different symbol in the library. */
	memcpy(small, "0123456789", 11);
	printf("memcpy %s\n", small);

	strcpy(small, "copied");
	printf("strcpy %s\n", small);

	strncpy(small, "truncated to fit", sizeof small - 1);
	small[sizeof small - 1] = '\0';
	printf("strncpy %s\n", small);

	written = snprintf(small, sizeof small, "%d-%s", 42, "ok");
	printf("snprintf %d %s\n", written, small);

	memmove(large + 1, large, 8);
	printf("memmove %d\n", large[0] == 'x' && large[8] == 'x');

	/* A write to standard output through the fortified read and write pair, which is where the
	 * newer levels add checks the older ones do not have. Flushed first, because everything above
	 * went through a buffer and this does not, and an ordering that depends on whether the output
	 * is a terminal is a test that fails on Tuesdays. */
	fflush(stdout);
	if (write(1, "write ok\n", 9) != 9) {
		printf("write failed\n");
		return 1;
	}

	printf("bounded %d\n", (int)strnlen(large, sizeof large));
	return 0;
}
