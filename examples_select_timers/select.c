//select.c open 2 pipes connected to spawned processes each sending date 
//data is read in various buffer sizes thus multiplexing the prints from each process
//AUTHOR Shoshan Dagany shoshan.dagany@gmail.com
//REFERENCE advanced unix programming
//
/* select.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>


char logging[90000];
char tmp[300];
static void
quit(int rc,const char *fmt,...) {
	va_list ap;

	if ( errno != 0 ) /* Report errno */
		fprintf(stderr,"%s: ",strerror(errno));

	va_start(ap,fmt); /* Format error message */
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fputc('\n',stderr);

	exit(rc); /* Exit with return code */
}

int
main(int argc,char **argv) {
	int z; /* General status code */
	int f1; /* Open fifo 1 */
	int f2; /* Open fifo 2 */
	fd_set rxset; /* Read fd set */
	int nfds; /* Number of file descriptors */
	struct timeval tv; /* Timeout */
	char buf[2000+1]; /* I/O Buffer */
	FILE *p1, *p2; /* Pipes from popen(3) */

   memset(logging,0,7000);
	/*
	* Pipes :
	*/
	//if ( !(p1 = popen("ls -l /usr/include |tr '[a-z]' '[A-Z]'","r")) )
	if ( !(p1 = popen("ls -l /usr/include ","r")) )
		quit(1,"popen(3) failed for p1");  

//	if ( !(p2 = popen("ls -ltr /usr/ |tr '[A-Z]' '[a-z]'&& sleep 8","r")) )
	if ( !(p2 = popen("ls  -1 /usr/include  && sleep 8","r")) )	
		quit(1,"popen(3) failed for p2"); 

	/*
	* Obtain the underlying file descriptors :
	*/
	f1 = fileno(p1);
	f2 = fileno(p2);
	printf("BEGUN: f1=%d, f2=%d\n",f1,f2);

	/*
	* Enter a select loop :
	*/
	do {
		FD_ZERO(&rxset); /* Clear set */
		if ( f1 >= 0 )
			FD_SET(f1,&rxset); /* Check f1 */
		if ( f2 >= 0 )
			FD_SET(f2,&rxset); /* Check f2 */

		nfds = (f1 > f2 ? f1 : f2) + 1;
		tv.tv_sec = 3; /* 3 seconds */
		tv.tv_usec = 500000; /* + 0.5 seconds */

		do {
			z = select(nfds,&rxset,0,0,&tv);
		} while ( z == -1 && errno == EINTR );

		if ( z == -1 ) /* Error? */
			quit(13,"select(2)");

		if ( z == 0 ) {
			sprintf (tmp,"TIMEOUT: f1=%d, f2=%d\n",f1,f2);
			strcat (logging,tmp);
			continue;
		}

		/*
		* Control is here if f1 or f2 has data
		* available to be read.
		*/
		if ( f1 >= 0 && FD_ISSET(f1,&rxset) ) {
			z = read(f1,buf,sizeof buf-1);
			if ( z == -1 )
				quit(6,"read(2) of f1.");
			if ( z > 0 ) {
				buf[z] = 0;
				sprintf(tmp,"\n*** read %d bytes  from f1 \n%s",z,buf);
				strcat(logging,tmp);
			} else {
				sprintf(tmp,"read EOF from f1;\n");
				strcat (logging,tmp);
				pclose(p1);
				f1 = -1;
			}
		}

		if ( f2 >= 0 && FD_ISSET(f2,&rxset) ) {
			z = read(f2,buf,200);
			if ( z == -1 )
				quit(6,"read(2) of f2.");
			if ( z > 0 ) {
				buf[z] = 0;
				sprintf(tmp,"\n*** read %d bytes  from f2\n%s ",z,buf);
				strcat(logging,tmp);
			} else {
				strcat (logging ,"read EOF from f2;\n");
				pclose(p2);
				f2 = -1;
			}
		}

	} while ( f1 >= 0 || f2 >= 0 ); 

	strcat(logging,"End select.");
    printf("%s",logging);
	return 0;
}