#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>





#if  __i386 || __i386__ || i386 || __arm || __arm__
#define USE_
/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

void printBackTraceHandler(int sig_num, siginfo_t * info, void * ucontext)
{
 void *             array[50];
 void *             caller_address;
 char **            messages;
 int                size, i;
 sig_ucontext_t *   uc;

 uc = (sig_ucontext_t *)ucontext;

 /* Get the address at the time the signal was raised from the EIP (x86) */
#if  __i386 || __i386__ || i386
 caller_address = (void *) uc->uc_mcontext.eip;   
#elif __arm || __arm__
 caller_address = (void *) uc->uc_mcontext.arm_pc;
#else
	#error  CPU_TYPE_NOT_HANDLED 
#endif
 fprintf(stderr, "signal %d (%s), address is %p from %p\n", 
  sig_num, strsignal(sig_num), info->si_addr, 
  (void *)caller_address);

 size = backtrace(array, 50);

 /* overwrite sigaction with caller's address */
 array[1] = caller_address;

 messages = backtrace_symbols(array, size);

 /* skip first stack frame (points here) */
 for (i = 1; i < size && messages != NULL; ++i)
 {
  fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
 }

 free(messages);

 exit(EXIT_FAILURE);
}


#else
void printBackTraceHandler(void)
{
	void* array[20];
	size_t size;

	fprintf(stderr, "\n BACKTRACE");
	size = backtrace( array, 20);
	backtrace_symbols_fd( array, 20, fileno(stderr));
}

#endif





void PrintStackTrace_init() {
	struct sigaction sigact;
	
	 sigact.sa_sigaction = printBackTraceHandler;
	 sigact.sa_flags = SA_RESTART | SA_SIGINFO;
	
	 if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0)
	 {
	  fprintf(stderr, "error setting signal handler for %d (%s)\n",
	    SIGSEGV, strsignal(SIGSEGV));
	
	  exit(EXIT_FAILURE);
	 }
 }