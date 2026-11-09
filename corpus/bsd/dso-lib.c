/* The library half of dso.c, built with -shared -fPIC.
 *
 * A variable in a shared object is in a block the loader places when it loads the object, so code
 * in the object cannot know its offset from the thread pointer. It asks __tls_get_addr, through a
 * pair of GOT slots the loader fills in, which is the general dynamic model. The program reaches
 * the same variable as an extern, through one GOT slot holding the offset, which is the initial
 * exec model and works because the object is loaded before the program starts.
 */

__thread int shared = 40;
static __thread int calls;

int bump(int by)
{
	calls++;
	shared += by;
	return shared;
}

int calls_here(void)
{
	return calls;
}
