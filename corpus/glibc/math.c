/* libm, which is where a symbol has more than one version node inside the window these releases
 * cover.
 *
 * glibc 2.29 gave pow, exp, log, exp2 and log2 new versions, because the old ones set errno in a
 * way the new ones do not, so a binary built against 2.28 names pow@GLIBC_2.2.5 and one built
 * against 2.39 names pow@GLIBC_2.29. Both exist in 2.39 and only the first exists in 2.28, and the
 * loader is what notices.
 *
 * Which node a reference gets is decided by the link rather than by the header, so this says less
 * about the merged tree than the other programs here and more about the release a target asked for
 * being carried all the way through. It is here because it is the cheapest program that fails when
 * that is not true, and because the header still decides whether a call is made at all: math.h
 * redirects several of these to __builtin_ forms and to the finite variants on the older releases.
 *
 * The values are ones with exact answers in double precision, so this is not a test of anybody's
 * rounding. A wrong result would mean the wrong function was called, which is the thing being
 * asked about.
 */

#include <math.h>
#include <stdio.h>

int main(void)
{
	/* Every argument is volatile, so each of these is a call rather than a number the compiler
	 * worked out. A folded call names no symbol, and naming the symbol is the whole point. */
	volatile double zero = 0.0;
	volatile double one = 1.0;
	volatile double two = 2.0;
	volatile double ten = 10.0;
	volatile double eight = 8.0;
	volatile double kilo = 1024.0;
	volatile double gross = 144.0;
	volatile double half = -0.5;

	printf("pow %d\n", pow(two, ten) == 1024.0);
	printf("exp %d\n", exp(zero) == 1.0);
	printf("log %d\n", log(one) == 0.0);
	printf("exp2 %d\n", exp2(eight) == 256.0);
	printf("log2 %d\n", log2(kilo) == 10.0);
	printf("sqrt %d\n", sqrt(gross) == 12.0);
	printf("fma %d\n", fma(two, one, zero) == 2.0);
	printf("fabs %d\n", fabs(half) == 0.5);

	/* The classification macros are header only and read the bits of a double directly, so they
	 * are the part of math.h that can be wrong without any symbol being involved. */
	printf("classify %d\n", isnan(zero / zero) && isinf(one / zero) && isfinite(one));
	printf("signbit %d\n", signbit(-0.0) != 0 && signbit(0.0) == 0);

	return 0;
}
