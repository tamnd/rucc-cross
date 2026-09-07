/* Aggregates across a call boundary, in the shapes the classification rules turn on.
 *
 * Every function here is deliberately not static and deliberately in one file, so that a compiler
 * that inlines the whole thing still has to agree with itself about the calling convention it
 * would have used. Compile with -O0 for the honest version of this test, which is what
 * bin/run-corpus does, and then again at -O2 to check the inlined answer matches.
 *
 * The output is the same text on every target. It has to be: the numbers are values rather than
 * sizes, so a target that prints something different has miscounted a register or a stack slot
 * rather than merely laid something out differently. That is what makes this file a differential
 * test rather than a record of what each target does.
 */

#include <stdio.h>

/* Two eightbytes of integer. Two general registers under System V, a hidden pointer under Win64,
 * and the pair that shows the two apart most cheaply. */
struct two_words {
	long first;
	long second;
};

/* One eightbyte, half integer and half float. System V merges the two halves into one class and
 * the answer is a general register, which is the merge rule and not an obvious guess. */
struct int_and_float {
	int count;
	float value;
};

/* A homogeneous aggregate of four floats. Four float registers under AAPCS64 and RISC-V, and no
 * such concept under System V, where it is two eightbytes of SSE. */
struct four_floats {
	float a, b, c, d;
};

/* Larger than any register bank will take, so it travels by memory on every ABI here. What differs
 * is whether the registers it did not fit into are still available to the next argument. */
struct big {
	long parts[8];
};

/* One float and one double, which is not homogeneous because the members are different types, and
 * is the case a scan that only counts floating point members gets wrong. */
struct mixed_floats {
	float narrow;
	double wide;
};

long sum_two_words(struct two_words value)
{
	return value.first + value.second;
}

long sum_int_and_float(struct int_and_float value)
{
	return value.count + (long)value.value;
}

long sum_four_floats(struct four_floats value)
{
	return (long)(value.a + value.b + value.c + value.d);
}

long sum_big(struct big value)
{
	long total = 0;
	for (int i = 0; i < 8; i++)
		total += value.parts[i];
	return total;
}

long sum_mixed_floats(struct mixed_floats value)
{
	return (long)(value.narrow + value.wide);
}

/* An aggregate after enough scalars to use the bank up. Under System V the aggregate goes to
 * memory and the ninth scalar still gets a register if one is left, which is the rule that says
 * running short does not drain. Under AAPCS64 it does drain. */
long late_aggregate(long a, long b, long c, long d, long e, struct two_words value, long f)
{
	return a + b + c + d + e + value.first + value.second + f;
}

/* The same aggregate returned rather than passed, which is a separate rule with a separate answer
 * on several of these ABIs. */
struct two_words make_two_words(long first, long second)
{
	struct two_words value = { first, second };
	return value;
}

struct four_floats make_four_floats(void)
{
	struct four_floats value = { 1.0f, 2.0f, 4.0f, 8.0f };
	return value;
}

int main(void)
{
	struct two_words two = { 3, 4 };
	struct int_and_float mixed = { 5, 6.0f };
	struct four_floats floats = { 1.0f, 2.0f, 3.0f, 4.0f };
	struct big big;
	struct mixed_floats narrow_wide = { 1.5f, 2.5 };

	for (int i = 0; i < 8; i++)
		big.parts[i] = i + 1;

	printf("two_words %ld\n", sum_two_words(two));
	printf("int_and_float %ld\n", sum_int_and_float(mixed));
	printf("four_floats %ld\n", sum_four_floats(floats));
	printf("big %ld\n", sum_big(big));
	printf("mixed_floats %ld\n", sum_mixed_floats(narrow_wide));
	printf("late_aggregate %ld\n", late_aggregate(1, 2, 3, 4, 5, two, 6));

	struct two_words returned = make_two_words(10, 20);
	printf("returned %ld %ld\n", returned.first, returned.second);

	struct four_floats floats_back = make_four_floats();
	printf("returned_floats %ld\n",
	       (long)(floats_back.a + floats_back.b + floats_back.c + floats_back.d));

	return 0;
}
