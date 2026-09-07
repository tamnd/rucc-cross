/* Variadic arguments, which is where an ABI stops looking like the fixed case.
 *
 * Three of the divergences in spec/cross-compile/06-abis.md live here. Darwin on arm64 puts every
 * variadic argument in the argument area rather than in a register, against what AAPCS64 says for
 * fixed arguments. System V on x86-64 requires al to hold the number of vector registers used,
 * which is a caller obligation with no fixed-argument equivalent. And a float promotes to a double
 * in a variadic call on every target, so a caller that passes a float has passed a double.
 *
 * The output is the same on every target for the same reason as aggregates.c: these are values,
 * not layouts.
 */

#include <stdarg.h>
#include <stdio.h>

/* Integers only. The simplest case and the one that has to work before anything else is worth
 * looking at. */
long sum_longs(int count, ...)
{
	va_list args;
	long total = 0;

	va_start(args, count);
	for (int i = 0; i < count; i++)
		total += va_arg(args, long);
	va_end(args);

	return total;
}

/* Doubles only, which under System V is where al matters. A caller that fails to set it produces a
 * callee that reads the wrong save area, and the failure is silent. */
double sum_doubles(int count, ...)
{
	va_list args;
	double total = 0;

	va_start(args, count);
	for (int i = 0; i < count; i++)
		total += va_arg(args, double);
	va_end(args);

	return total;
}

/* The two interleaved, which is the case where the two save areas advance independently and a
 * naive implementation that keeps one cursor gets the third argument wrong. */
double sum_mixed(int count, ...)
{
	va_list args;
	double total = 0;

	va_start(args, count);
	for (int i = 0; i < count; i++) {
		total += (double)va_arg(args, long);
		total += va_arg(args, double);
	}
	va_end(args);

	return total;
}

/* Enough arguments to exhaust any register bank and spill onto the stack, so that the transition
 * from the save area to the overflow area is exercised rather than assumed. */
long sum_many(int count, ...)
{
	va_list args;
	long total = 0;

	va_start(args, count);
	for (int i = 0; i < count; i++)
		total += va_arg(args, long);
	va_end(args);

	return total;
}

/* A copy taken partway through and read to the end, which is what every printf implementation does
 * and the operation an incorrect va_list layout breaks first. */
long sum_with_copy(int count, ...)
{
	va_list args;
	va_list again;
	long first = 0;
	long second = 0;

	va_start(args, count);
	first += va_arg(args, long);
	va_copy(again, args);
	for (int i = 1; i < count; i++)
		second += va_arg(args, long);
	va_end(args);

	long copied = 0;
	for (int i = 1; i < count; i++)
		copied += va_arg(again, long);
	va_end(again);

	return first + second + (copied == second ? 0 : 1000);
}

int main(void)
{
	printf("longs %ld\n", sum_longs(4, 1L, 2L, 3L, 4L));
	printf("doubles %.3f\n", sum_doubles(4, 1.0, 2.0, 4.0, 8.0));
	printf("mixed %.3f\n", sum_mixed(3, 1L, 0.5, 2L, 0.25, 3L, 0.125));
	printf("many %ld\n",
	       sum_many(12, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L, 10L, 11L, 12L));
	printf("copy %ld\n", sum_with_copy(5, 1L, 2L, 3L, 4L, 5L));

	/* A float argument in a variadic call is a double by the time the callee sees it, so
	 * reading it as a float would read half of one. */
	float narrow = 1.5f;
	printf("promoted %.3f\n", sum_doubles(1, narrow));

	return 0;
}
