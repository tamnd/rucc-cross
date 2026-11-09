/* Constructors and destructors, which run from .init_array and .fini_array.
 *
 * Neither system's start file calls a function named in those arrays by itself. The program's
 * own copy of crtbegin.o, the loader for a dynamic program and libc for a static one each walk an
 * array, so a constructor that does not run, or runs twice, is a section the linker put in the
 * wrong place or an entry the start files did not expect. The priorities decide the order inside
 * the arrays: the lower number runs first and its destructor last.
 *
 * Every line goes out through write(2), because a destructor runs after main and the order of the
 * lines is the result. stdio would put them in one buffer and print them in the order they were
 * written, which is the same, but a buffer that is never flushed would lose them all at once.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int constructed;

static void say(const char *line)
{
	write(1, line, strlen(line));
	write(1, "\n", 1);
}

__attribute__((constructor(101))) static void first(void)
{
	constructed++;
	say("constructor 101");
}

__attribute__((constructor(102))) static void second(void)
{
	constructed++;
	say("constructor 102");
}

__attribute__((constructor)) static void plain(void)
{
	constructed++;
	say("constructor");
}

__attribute__((destructor(101))) static void last(void)
{
	say("destructor 101");
}

__attribute__((destructor(102))) static void before_last(void)
{
	say("destructor 102");
}

__attribute__((destructor)) static void plain_end(void)
{
	say("destructor");
}

static void registered(void)
{
	say("atexit");
}

int main(void)
{
	say(constructed == 3 ? "main after 3 constructors" : "main after the wrong number of constructors");
	atexit(registered);
	return 0;
}
