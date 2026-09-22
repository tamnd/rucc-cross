/* sigaction and sigset_t, where the header and the library have to agree about a bitmap.
 *
 * sigset_t is an array of words with no accessor, and every one of sigemptyset, sigaddset and
 * sigismember is a macro or an inline in the header for the common cases. So the program decides
 * where the bits go and the library decides where it reads them, and if those two came from
 * different releases the mask that gets installed is not the mask that was asked for. There is no
 * error for that. The signal is simply not blocked, or a different one is.
 *
 * struct sigaction has the same shape of problem in a worse place, because sa_handler and
 * sa_sigaction are a union in some spellings and the flags field decides which one the kernel will
 * use. A handler read out of the wrong offset is a jump to whatever was next to it.
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

static volatile sig_atomic_t caught;
static volatile sig_atomic_t code;

static void handler(int number, siginfo_t *info, void *context)
{
	(void)context;
	caught = number;
	code = info->si_signo;
}

int main(void)
{
	struct sigaction action;
	struct sigaction previous;
	sigset_t blocked;
	sigset_t pending;

	memset(&action, 0, sizeof action);
	action.sa_sigaction = handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGUSR1, &action, &previous) != 0) {
		printf("sigaction failed\n");
		return 1;
	}

	/* Blocked, raised, and then asked whether it is pending. The kernel answers that out of a
	 * sigset_t it filled in, so reading it back with sigismember is the header and the library
	 * agreeing about the same bitmap from opposite ends. */
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &blocked, NULL) != 0) {
		printf("sigprocmask failed\n");
		return 1;
	}

	raise(SIGUSR1);
	sigemptyset(&pending);
	if (sigpending(&pending) != 0) {
		printf("sigpending failed\n");
		return 1;
	}
	printf("pending %d\n", sigismember(&pending, SIGUSR1) == 1);
	printf("quiet %d\n", caught == 0);

	/* Unblocking delivers it, and the handler runs before sigprocmask returns. */
	if (sigprocmask(SIG_UNBLOCK, &blocked, NULL) != 0) {
		printf("sigprocmask failed\n");
		return 1;
	}
	printf("caught %d\n", caught == SIGUSR1);
	printf("info %d\n", code == SIGUSR1);

	/* SIGKILL is the one signal that cannot be blocked, and glibc filters it out of the mask
	 * rather than letting the kernel refuse, so this asks the header's own bookkeeping. */
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGKILL);
	printf("kill %d\n", sigismember(&blocked, SIGKILL) == 1);

	return 0;
}
