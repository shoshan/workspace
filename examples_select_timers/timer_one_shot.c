//from advanced unix programming
//TODO check
 /* r1shot.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
static int count = 0;               /* Counter */
static int rtime=0;
/*
 * Signal handler :
 */
static void
handler(int signo) {
    int e = errno;                  /* Save errno */
    ++count;                        /* Increment count */
    write(1,"<<<SIGALRM>>>\n",14);
    errno = e;                      /* Restore errno */
}
/*
 * Main program :
 */
int
main(int argc,char **argv) {
    int z;                          /* Status return code */
    struct sigaction new_sigalrm;   /* New signal action */
    struct itimerval old_timer;     /* Old timer values */
    struct itimerval new_timer;     /* New timer values */
    /*
     * Establish the signal action required for SIGALRM :
     */
    new_sigalrm.sa_handler = handler;
    sigemptyset(&new_sigalrm.sa_mask);
    new_sigalrm.sa_flags = 0;
    sigaction(SIGALRM,&new_sigalrm,NULL);
    /*
     * Establish a one-shot realtime timer :
     */
    new_timer.it_interval.tv_sec = 0;
    new_timer.it_interval.tv_usec = 0;
    new_timer.it_value.tv_sec = 5;
    new_timer.it_value.tv_usec = 250000;    /* 5.25 seconds */
    puts("Starting ITIMER_REAL...");
    z = setitimer(ITIMER_REAL,&new_timer,&old_timer);
    if ( z ) {
    	      perror("setitimer(ITIMER_REAL)");
      return 1;
    }
    /*
     * A loop :
     */
    do {
         /* Do Work...*/ ;
         printf("rtime=%d\n",rtime);
         sleep(1);
         rtime++;
         
       } while ( count < 1 );
       printf("ITIMER_REAL count is %d.\n",count);
  return 0;
}
    	
