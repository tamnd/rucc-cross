/* Thread local storage in the program itself, which each BSD lays out its own way.
 *
 * x86-64 puts the block of the program below the thread pointer, at an offset the linker works
 * out from the size and alignment of the whole segment, and the code reaches a variable with a
 * negative offset from %fs. The offset is right only when the compiler, the linker and the
 * system's loader agree about where the block starts, so a mistake shows up as a thread reading
 * another variable or another thread's copy rather than as a crash.
 *
 * Three variables, so that the layout is more than one slot: an int with a value, an array with
 * nothing in it, which goes in .tbss, and one aligned to 64, which moves the start of the block.
 * Each thread writes its own number into all three and reads them back after the others have
 * written theirs.
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define THREADS 4

static __thread int counter = 7;
static _Thread_local char name[16];
static _Thread_local _Alignas(64) long wide = -1;

static pthread_barrier_t everyone;
static int counters[THREADS];
static int names[THREADS];
static int wides[THREADS];
static int aligned[THREADS];

static void *worker(void *argument)
{
	int number = (int)(long)argument;
	char want[16];

	/* The value from the template, before this thread writes anything. */
	int fresh = counter == 7 && name[0] == '\0' && wide == -1;

	counter += number;
	snprintf(name, sizeof name, "thread %d", number);
	wide = number * 1000L;

	/* Every thread has written before any thread reads. */
	pthread_barrier_wait(&everyone);

	snprintf(want, sizeof want, "thread %d", number);
	counters[number] = fresh ? counter : -1;
	names[number] = strcmp(name, want) == 0;
	wides[number] = wide == number * 1000L;
	aligned[number] = ((unsigned long)&wide & 63) == 0;
	return NULL;
}

int main(void)
{
	pthread_t threads[THREADS];
	int i;

	pthread_barrier_init(&everyone, NULL, THREADS);
	for (i = 0; i < THREADS; i++) {
		if (pthread_create(&threads[i], NULL, worker, (void *)(long)i) != 0) {
			printf("pthread_create failed\n");
			return 1;
		}
	}
	for (i = 0; i < THREADS; i++)
		pthread_join(threads[i], NULL);
	pthread_barrier_destroy(&everyone);

	for (i = 0; i < THREADS; i++)
		printf("thread %d counter %d name %d wide %d aligned %d\n", i, counters[i], names[i], wides[i], aligned[i]);

	/* The main thread's own copies were never written. */
	printf("main counter %d name %d wide %d\n", counter, name[0] == '\0', wide == -1);
	return 0;
}
