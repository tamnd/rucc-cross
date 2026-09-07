/* What every target must agree about its scalar types, and the four places they stop agreeing.
 *
 * bin/facts reads the type widths out of the preprocessor, so nothing here repeats them. What is
 * here is alignment, which the preprocessor does not answer, and it is written as assertions
 * rather than as recorded numbers on purpose. An alignment is a rule with a reason, and
 * `_Alignof(double) == 4` on i386 next to the sentence saying why is worth more than a 4 in a
 * generated file.
 *
 * This compiles for every target in the table with no libc and no linking, which is what makes it
 * usable for the freestanding rows as well as the hosted ones.
 */

/* The floor. A scalar is aligned to its own size or less, never to more, and the smallest ones are
 * aligned to themselves everywhere. Nothing in the table has ever violated this and a target that
 * did would break every struct layout rule downstream of it. */
_Static_assert(_Alignof(char) == 1, "a char is a byte and a byte is aligned to itself");
_Static_assert(_Alignof(short) == sizeof(short), "a short aligns to its size on every target");
_Static_assert(_Alignof(int) == sizeof(int), "an int aligns to its size on every target");
_Static_assert(_Alignof(float) == sizeof(float), "a float aligns to its size on every target");
_Static_assert(_Alignof(void *) == sizeof(void *), "a pointer aligns to its size on every target");

/* The first place they differ. System V on i386 aligns an eight byte scalar to four, because the
 * 1990 psABI said so and every struct laid out since has depended on it. This is the reason a data
 * layout carries an alignment beside a size instead of deriving one from the other.
 *
 * Windows on i386 does not. Microsoft aligned a double to eight and mingw-w64 follows Microsoft,
 * so `__i386__` is not the condition and `__i386__ && !_WIN32` is. That is a divergence between
 * two targets that share an architecture and a word size, found by this file failing to compile
 * for i686-windows-gnu when it was first written against the architecture alone. */
#if defined(__i386__) && !defined(_WIN32)
_Static_assert(_Alignof(double) == 4, "System V on i386 aligns a double to four, not to eight");
_Static_assert(_Alignof(long long) == 4, "and a long long to four as well");
#else
_Static_assert(_Alignof(double) == sizeof(double), "everywhere else a double aligns to its size");
_Static_assert(_Alignof(long long) == sizeof(long long), "and so does a long long");
#endif

/* The second. A `long double` is four different types wearing one name, and the mantissa digits
 * are what tell them apart. The width alone does not: an IEEE quad and a PowerPC double double are
 * both sixteen bytes and they agree about no bit in either of them.
 *
 * 53 is a plain double, which is Windows, Darwin and 32-bit ARM.
 * 64 is the x87 eighty bit extended, padded to twelve bytes on i386 and sixteen on x86-64.
 * 106 is the PowerPC double double, which is a pair of doubles and not an IEEE format at all.
 * 113 is IEEE quad, which is AArch64 Linux, RISC-V, s390x, LoongArch and Android on x86-64. */
#if __LDBL_MANT_DIG__ == 53
_Static_assert(sizeof(long double) == 8, "a long double that is a double is eight bytes");
_Static_assert(_Alignof(long double) == _Alignof(double), "and aligns like one");
#elif __LDBL_MANT_DIG__ == 64
_Static_assert(sizeof(long double) == 12 || sizeof(long double) == 16,
	"an x87 long double is eighty bits of value in twelve or sixteen bytes of storage");
_Static_assert(sizeof(long double) > 10, "and the padding is storage, not value");
#elif __LDBL_MANT_DIG__ == 106
_Static_assert(sizeof(long double) == 16, "a double double is two doubles");
_Static_assert(_Alignof(long double) == 16, "aligned as a unit rather than as its halves");
#elif __LDBL_MANT_DIG__ == 113
_Static_assert(sizeof(long double) == 16, "an IEEE quad is sixteen bytes");
/* Aligned to itself everywhere except s390x, whose ELF ABI caps scalar alignment at eight and so
 * gives a sixteen byte type an eight byte alignment. That is the one target where size and
 * alignment come apart for a type nobody expects it of. */
#if defined(__s390x__)
_Static_assert(_Alignof(long double) == 8, "s390x caps alignment at eight, even for a quad");
#else
_Static_assert(_Alignof(long double) == 16, "aligned to itself");
#endif
#else
#error "a fifth long double format, which is a finding rather than a failure"
#endif

/* The third. Plain `char` has a sign and nobody agrees what it is, which is the oldest portability
 * bug in C and still the one that ships. It is checked here rather than only recorded in a facts
 * file because the two interesting pairs are pairs of targets that differ in exactly one field.
 *
 * aarch64-linux says unsigned, per AAPCS64. aarch64-macos says signed, because Apple kept it that
 * way for source compatibility with the Intel Macs. Same architecture, opposite answers, and the
 * only difference between the two tuples is the operating system.
 *
 * The other pair is aarch64-linux and aarch64-windows, where Windows says signed because the
 * Microsoft ABI does. */
#if defined(__CHAR_UNSIGNED__)
_Static_assert((char)-1 > 0, "__CHAR_UNSIGNED__ is defined, so a char had better be unsigned");
#else
_Static_assert((char)-1 < 0, "__CHAR_UNSIGNED__ is undefined, so a char had better be signed");
#endif
_Static_assert(sizeof(char) == 1, "a char is one byte by definition, whatever its sign");

/* The fourth. `__int128` is sixteen bytes wherever it exists and it is usually aligned to itself,
 * which is twice the word on every target that has it. s390x is the exception again, for the same
 * reason as the quad above.
 *
 * Two things this file was wrong about when it was written and the reference corrected. It is not
 * a 64-bit only type: wasm32 and x86_64-linux-gnux32 both have it with four byte pointers, which
 * is a good reminder that the pointer width and the widest integer are separate facts. And its
 * alignment is not always sixteen. */
#if defined(__SIZEOF_INT128__)
_Static_assert(sizeof(__int128) == 16, "an __int128 is sixteen bytes wherever it exists");
#if defined(__s390x__)
_Static_assert(_Alignof(__int128) == 8, "s390x caps this one at eight too");
#else
_Static_assert(_Alignof(__int128) == 16, "and it is aligned to itself, not to the word");
#endif
#endif

/* The integer types keep their order and their relationship to the pointer, which is what a data
 * model is. LP64, LLP64 and ILP32 are three answers to this and the table has all three. */
_Static_assert(sizeof(short) <= sizeof(int), "short is not wider than int");
_Static_assert(sizeof(int) <= sizeof(long), "int is not wider than long");
_Static_assert(sizeof(long) <= sizeof(long long), "long is not wider than long long");
_Static_assert(sizeof(long long) >= 8, "a long long is at least sixty four bits");
_Static_assert(sizeof(void *) == sizeof(void (*)(void)),
	"a data pointer and a function pointer are the same width on every target here");

/* Nothing above needs a main, and the freestanding rows have no libc to give it one. The file is
 * compiled with -c and the empty translation unit is the point. */
