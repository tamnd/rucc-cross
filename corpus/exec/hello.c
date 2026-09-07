/* The smallest thing that proves a sysroot, a link line and an emulator all work at once.
 *
 * This is rung 0 of the ladder in spec/cross-compile/14-testing.md, and its whole job is to fail
 * for a reason that is easy to read. If this does not run then nothing after it is worth
 * debugging, because the failure is in the toolchain rather than in the code generator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	(void)argv;

	/* A libc call that touches the heap, a libc call that touches string handling, and a libc
	 * call that touches stdio. Three different parts of the library, so that a sysroot missing
	 * one of them says which. */
	char *copy = malloc(32);
	if (copy == NULL)
		return 1;

	strcpy(copy, "hello from the corpus");
	printf("%s\n", copy);
	printf("argc %d\n", argc);
	free(copy);

	return 0;
}
