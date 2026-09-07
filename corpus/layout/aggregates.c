/* Struct and union layout, which is where a wrong alignment turns into a wrong offset.
 *
 * Everything here follows from the scalar alignments in scalars.c, and that is the point: if a
 * compiler gets the scalars right and these wrong then it has a layout bug rather than a table
 * bug, and the two are worth telling apart before anyone goes looking.
 */

/* The basic rule. A member sits at the next offset its own alignment allows, the struct is aligned
 * to its most aligned member, and the size is rounded up so that an array of them keeps every
 * element aligned. */
struct pair {
	char first;
	int second;
};
_Static_assert(_Alignof(struct pair) == _Alignof(int), "a struct is as aligned as its worst member");
_Static_assert(sizeof(struct pair) == 2 * sizeof(int), "and padded so an array of it still lines up");
_Static_assert(__builtin_offsetof(struct pair, second) == sizeof(int), "the int skips to its alignment");

/* Trailing padding is part of the size, and it is what makes `sizeof` a multiple of the alignment
 * rather than the offset of the last byte anybody wrote. This is the one that produces a wrong
 * answer at a call boundary rather than a wrong answer in memory, because an ABI classifies on the
 * size and the size includes padding nobody stored to. */
struct trailing {
	int wide;
	char narrow;
};
_Static_assert(sizeof(struct trailing) == 2 * sizeof(int), "the tail is padded to the alignment");
_Static_assert(__builtin_offsetof(struct trailing, narrow) == sizeof(int), "with nothing before it");

/* i386 again, and this is the case the alignment divergence actually shows up in. A struct of an
 * int and a double is twelve bytes under System V and sixteen everywhere else, including on
 * Windows i386, so the same source produces two different objects and two different
 * classifications for two targets that share an architecture. */
struct mixed {
	int count;
	double value;
};
#if defined(__i386__) && !defined(_WIN32)
_Static_assert(sizeof(struct mixed) == 12, "System V on i386 packs the double at four");
_Static_assert(__builtin_offsetof(struct mixed, value) == 4, "so the double follows immediately");
#else
_Static_assert(sizeof(struct mixed) == 16, "everywhere else the double wants eight");
_Static_assert(__builtin_offsetof(struct mixed, value) == 8, "so four bytes of padding go in first");
#endif

/* An empty struct is not C, so the smallest aggregate is one byte and it stays one byte. This
 * matters because several ABIs have a rule for an aggregate of size zero and C never produces one
 * from a struct declaration, only from a zero length array extension. */
struct one {
	char only;
};
_Static_assert(sizeof(struct one) == 1, "the smallest struct is a byte");
_Static_assert(_Alignof(struct one) == 1, "and it is aligned like the byte in it");

/* A union is as large as its largest member and as aligned as its most aligned one, and those can
 * be two different members. That is the case where a naive layout gets the size right and the
 * alignment wrong. */
union either {
	char bytes[9];
	int number;
};
_Static_assert(_Alignof(union either) == _Alignof(int), "the alignment comes from the int");
_Static_assert(sizeof(union either) == 12, "and the size is the array rounded up to it");

/* Nesting does not flatten. An inner struct keeps its own alignment inside the outer one, so the
 * outer one inherits it, and an ABI that scans members recursively has to see the same thing. */
struct outer {
	char lead;
	struct mixed inner;
};
_Static_assert(_Alignof(struct outer) == _Alignof(struct mixed), "the inner alignment propagates out");
_Static_assert(__builtin_offsetof(struct outer, inner) == _Alignof(struct mixed),
	"and the inner struct starts where its own alignment says");

/* An array of aggregates has no padding between elements, because the element size already
 * includes the trailing padding that keeps the next one aligned. Anything else and pointer
 * arithmetic stops agreeing with indexing. */
_Static_assert(sizeof(struct mixed[4]) == 4 * sizeof(struct mixed), "arrays are contiguous");
_Static_assert(_Alignof(struct mixed[4]) == _Alignof(struct mixed), "and no more aligned than one");

/* A flexible array member contributes nothing to the size and may contribute to the alignment.
 * Both halves matter: the size is what an ABI classifies on and the alignment is what the
 * allocation has to satisfy. */
struct flexible {
	int count;
	double values[];
};
_Static_assert(_Alignof(struct flexible) == _Alignof(double), "the flexible member still aligns the struct");
_Static_assert(sizeof(struct flexible) <= sizeof(struct mixed), "and adds nothing to the size");

/* An over-aligned member drags the whole struct up with it, past anything the ABI would have asked
 * for on its own. This is where a classifier that assumes the natural alignment of the widest
 * scalar goes wrong. */
struct wide {
	_Alignas(32) char payload[32];
};
_Static_assert(_Alignof(struct wide) == 32, "_Alignas raises the struct alignment");
_Static_assert(sizeof(struct wide) == 32, "without changing the size here");
