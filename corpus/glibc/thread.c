/* Threads, which glibc 2.34 moved out of libpthread and into libc.
 *
 * Before 2.34 a program that called pthread_create had to link against libpthread, and the header
 * decided nothing about that: the link line did. From 2.34 on the symbols are in libc and
 * libpthread is an empty compatibility stub that exists so old link lines keep working. A tree that
 * merges eight releases has to keep working in both directions, and what settles it is not the
 * header but whether the binary loads and the thread runs.
 *
 * The mutex and the condition variable are here because their initializers are macros expanding to
 * a brace list of the internal structure, so their size and field order come from the header while
 * the code that locks them comes from the library. That is the same class of mismatch as stat and
 * it fails in the same quiet way: a static initializer from one release and an implementation from
 * another agree on nothing in particular and the program deadlocks or scribbles.
 */

#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ready = PTHREAD_COND_INITIALIZER;
static int counter;
static int started;

static void *worker(void *argument)
{
	int rounds = *(int *)argument;
	int i;

	pthread_mutex_lock(&lock);
	started = 1;
	pthread_cond_signal(&ready);
	pthread_mutex_unlock(&lock);

	for (i = 0; i < rounds; i++) {
		pthread_mutex_lock(&lock);
		counter++;
		pthread_mutex_unlock(&lock);
	}
	return argument;
}

int main(void)
{
	pthread_t threads[4];
	int rounds = 1000;
	void *result;
	int i;

	for (i = 0; i < 4; i++) {
		if (pthread_create(&threads[i], NULL, worker, &rounds) != 0) {
			printf("pthread_create failed\n");
			return 1;
		}
	}

	/* The condition variable is waited on rather than slept on, so that this says something about
	 * the condition variable rather than about the scheduler. */
	pthread_mutex_lock(&lock);
	while (started == 0)
		pthread_cond_wait(&ready, &lock);
	pthread_mutex_unlock(&lock);

	for (i = 0; i < 4; i++) {
		if (pthread_join(threads[i], &result) != 0 || result != &rounds) {
			printf("pthread_join failed\n");
			return 1;
		}
	}

	printf("counter %d\n", counter);
	printf("self %d\n", pthread_equal(pthread_self(), pthread_self()) != 0);
	return 0;
}
