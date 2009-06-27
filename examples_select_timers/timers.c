/* timers.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
static int count = 0;                /* Counter */

 /*
   * Signal handler :
   */
 static void
 handler(int signo) {
      int e = errno;                  /* Save errno */
      char *signame = "?";            /* Signal name */

      switch( signo ) {
      case SIGALRM :                  /* Realtime timer expired */
          ++count;                    /* Increment counter */
          signame = "<<<SIGALRM>>>\n";
          break;
      case SIGVTALRM :                /* Virtual timer expired */
          signame = "<<<SIGVTALRM>>>\n";
          break;
      case SIGPROF :                  /* System virtual timer expired */
          signame = "<<<SIGPROF>>>\n";
          break;
      }

      write(1,signame,strlen(signame));
      errno = e;                      /* Restore errno */
 }

 /*
   * Main program :
   */
 int
 main(int argc,char **argv) {
      int z;                          /* Status return code */
      struct sigaction new_sigalrm;   /* New signal action */
      struct itimerval real_timer;    /* Real timer values */
      struct itimerval virt_timer;    /* User mode timer */
      struct itimerval prof_timer;    /* System+User mode timer */
      struct itimerval timer_values; /* Timer values */

      /*
       * Establish the signal action required for SIGALRM :
       */
      new_sigalrm.sa_handler = handler;
      sigemptyset(&new_sigalrm.sa_mask);
      new_sigalrm.sa_flags = 0;

      sigaction(SIGALRM,&new_sigalrm,NULL);
      sigaction(SIGVTALRM,&new_sigalrm,NULL);
      sigaction(SIGPROF,&new_sigalrm,NULL);

      /*
       * Establish a realtime timer :
       */
      real_timer.it_interval.tv_sec = 3;
      real_timer.it_interval.tv_usec = 500000; /* 3.5 seconds */
real_timer.it_value.tv_sec = 3;
real_timer.it_value.tv_usec = 500000;
virt_timer.it_interval.tv_sec = 0;
virt_timer.it_interval.tv_usec = 500000; /* 0.5 seconds */
virt_timer.it_value.tv_sec = 0;
virt_timer.it_value.tv_usec = 500000;
prof_timer.it_interval.tv_sec = 0;
prof_timer.it_interval.tv_usec = 500000; /* 0.5 seconds */
prof_timer.it_value.tv_sec = 0;
prof_timer.it_value.tv_usec = 500000;
puts("Starting ITIMER_REAL...");
z = setitimer(ITIMER_REAL,&real_timer,NULL);
if ( z ) { 
    perror("setitimer(ITIMER_REAL)");
    return 1;
}
puts("Starting ITIMER_VIRTUAL...");
z = setitimer(ITIMER_VIRTUAL,&virt_timer,NULL);
if ( z ) {
    perror("setitimer(ITIMER_VIRTUAL)");
    return 1;
}
puts("Starting ITIMER_PROF...");
z = setitimer(ITIMER_PROF,&prof_timer,NULL);
if ( z ) {
    perror("setitimer(ITIMER_PROF)");
    return 1;
}
/*
 * A loop :
 */
do {
    /* Perform work which involves system time */
    getitimer(ITIMER_PROF,&timer_values);
    (void) timer_values;
} while ( count < 2 );
printf("ITIMER_REAL count is %d.\n",count);
return 0;
}