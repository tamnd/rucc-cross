/* Thread local storage in a shared object, read from the object and from the program.
 *
 * dso-lib.c is the object. Each thread changes its own copy of `shared` through a call into the
 * object and then reads it directly, so the two models of dso-lib.c have to name the same slot for
 * the same thread. The object is found next to the program through the $ORIGIN run path, which
 * both systems' loaders read.
 */

#include <pthread.h>
#include <stdio.h>

#define THREADS 3

extern __thread int shared;
int bump(int by);
int calls_here(void);

static int seen[THREADS];
static int direct[THREADS];
static int calls[THREADS];

static void *worker(void *argument)
{
	int number = (int)(long)argument;

	bump(number);
	seen[number] = bump(number);
	direct[number] = shared;
	calls[number] = calls_here();
	return NULL;
}

int main(void)
{
	pthread_t threads[THREADS];
	int i;

	for (i = 0; i < THREADS; i++) {
		if (pthread_create(&threads[i], NULL, worker, (void *)(long)i) != 0) {
			printf("pthread_create failed\n");
			return 1;
		}
	}
	for (i = 0; i < THREADS; i++)
		pthread_join(threads[i], NULL);

	for (i = 0; i < THREADS; i++)
		printf("thread %d bump %d direct %d calls %d\n", i, seen[i], direct[i], calls[i]);
	printf("main shared %d calls %d\n", shared, calls_here());
	return 0;
}
