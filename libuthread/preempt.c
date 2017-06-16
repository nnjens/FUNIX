#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define _UTHREAD_PRIVATE
#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100


void preempt_save(sigset_t *level)
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, level); //Sets SIGVTALRM to be blocked, saves previous  in level
}

void preempt_restore(sigset_t *level)
{
	sigprocmask(SIG_SETMASK, level, NULL); //Overwrites with mask passed in level
}

void preempt_enable(void)
{
	struct sigaction sa, old;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &sa.sa_mask, &old.sa_mask); //Unblocks timer alarm
}

void preempt_disable(void)
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL); //Blocks timer alarm
}

bool preempt_disabled(void)
{
	sigset_t tmp, current;
	sigemptyset(&tmp);
	sigprocmask(SIG_BLOCK, &tmp, &current); //Blocks an empty set, saving current sigset in current
	return sigismember(&current, SIGVTALRM);	
}

/*
 * timer_handler - Timer signal handler (aka interrupt handler)
 * @signo - Received signal number (can be ignored)
 */
static void timer_handler(int signo)
{
	uthread_yield();
}

void preempt_start(void)
{
	struct sigaction sa;
	struct itimerval it;

	/*
	 * Install signal handler @timer_handler for dealing with alarm signals
	 */
	sa.sa_handler = timer_handler;
	sigemptyset(&sa.sa_mask);
	/* Make functions such as read() or write() to restart instead of
	 * failing when interrupted */
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGVTALRM, &sa, NULL)) {
		perror("sigaction");
		exit(1);
	}

	/*
	 * Configure timer to fire alarm signals at a certain frequency
	 */
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 1000000 / HZ;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 1000000 / HZ;
	if (setitimer(ITIMER_VIRTUAL, &it, NULL)) {
		perror("setitimer");
		exit(1);
	}
}

