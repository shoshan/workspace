//lab_multiple_server.c handle multiple clients command , clints can disconect or run continusly
//AUTHOR shoshan.dagany@gmail.com
//REFERENCES embedded linux course gby--github.com 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <signal.h>
#define _GNU_SOURCE
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h> 
#define SOCKETNAME "/tmp/lab_socket"
#define MSGLEN (100)

#define MYNAME "demo_server"
#define MAX_CLIENTS 50 

 
/* Forward declerations */
void error (char *);
void close_server (void);
void signal_handler (int sig);
void pipe_handler (int sig);
void* get_messeges_func ();
/* This stream handle will be our log file stresm */
FILE * logfile = NULL;

/*This integer counts seconds*/ 
int counter=1000000;
/* This boolean flag is used to signal a timer expired */
int count_pending=0;
/* This boolean flag is used to signal we wish to terminate */
int term_pending = 0;
 
 
 
int main (int argc, char *argv[])
{
   /* Tell them what's going to happen */
  printf("%s going into background... \n", MYNAME);
 
  /* Fork into the background and become a daemon*/
  //daemon(0,0);
 
  /* Open the log file */
  logfile = fopen("/tmp/log.txt", "w");
    if(!logfile) {
      exit(1);
    }
  
  /* Register a destructor function to be called at exit */
  atexit (close_server);
  /* register a handler for some exceptions */
  signal (SIGTERM, signal_handler);
  signal (SIGINT, signal_handler);
  /* We don't want to crap out if someone press CTRL_C during
the time the client is connected to us, so mask out
SIGPIPE */
  signal (SIGPIPE, SIG_IGN);
   struct sigaction new_sigalrm;   /* New signal action */
   struct itimerval real_timer;    /* Real timer values */
   int z;
      /*
       * Establish the signal action required for SIGALRM :
       */
      new_sigalrm.sa_handler = signal_handler;
      sigemptyset(&new_sigalrm.sa_mask);
      new_sigalrm.sa_flags = 0;

      sigaction(SIGALRM,&new_sigalrm,NULL);
      
      /*
       * Establish a realtime timer :
       */
      real_timer.it_interval.tv_sec = 1;
      real_timer.it_interval.tv_usec = 0; 
      real_timer.it_value.tv_sec = 1;
      real_timer.it_value.tv_usec = 0;
      z = setitimer(ITIMER_REAL,&real_timer,NULL);
      if ( z ) { 
         perror("setitimer(ITIMER_REAL)");
         return 1;
      }
 
  
  get_messeges_func();
  return 0;
}
 
int max_of(int listenfd, int clients_fds[]) {
	int max_fd;
	int i;

	max_fd = listenfd;
	for(i = 0; i < MAX_CLIENTS; ++i) {
		if (clients_fds[i] > max_fd) {
			max_fd = clients_fds[i];
		}
	}
	return max_fd;
}

int find_free_cell(int client[]) {
	int i;
	for (i = 0; i < MAX_CLIENTS; ++i) {
		if (client[i] == -1) {
			return i;
		}
	}
	return -1;
}
 
////////////////////////////////////////////////////////////////////////////
void* get_messeges_func ()
{
 
 
 
    int index, fd_max, fd_socket;
	int number_of_bits, n_fd_ready, fds_clients[MAX_CLIENTS];
	fd_set descriptors_set;
	
 
 
  /* socket stuff */
  int fd_listen_socket, fd_new_connection, servlen;
  unsigned int client_address_length;
  struct sockaddr_un client_address, serv_addr;
  /* buffer to get messages into */
  char buf[MSGLEN];
  /* Buffer to send messages. We could re-use buf,but it's ugly */
  char msg[MSGLEN];
  /* Raise this flag to quit */
  int quit = 0;
  struct timeval tv;
  
  
  int msg_counter=0;
   bzero(msg, sizeof(buf));
 
  /* Get a socket and bind to it */
  if ((fd_listen_socket = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    error ("creating socket");
  }
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy (serv_addr.sun_path, SOCKETNAME);
  servlen = strlen (serv_addr.sun_path) + sizeof (serv_addr.sun_family);
 
  if (bind (fd_listen_socket, (struct sockaddr *) &serv_addr, servlen) < 0)
    error ("binding socket");
 
 
    /* Wait for SIGTERM or connection */
 	listen(fd_listen_socket, 5);
	/* initialization */
	for(index = 0; index < MAX_CLIENTS; ++index)
	{
		fds_clients[index] = -1;
	}
	while (!quit)
	{

			/* Preparing rset */
		FD_ZERO(&descriptors_set);
		FD_SET(fd_listen_socket, &descriptors_set);
int max_of(int listenfd, int clients_fds[]) {
	int max_fd;
	int i;

	max_fd = listenfd;
	for(i = 0; i < MAX_CLIENTS; ++i) {
		if (clients_fds[i] > max_fd) {
			max_fd = clients_fds[i];
		}
	}
	return max_fd;
}

int find_free_cell(int client[]) {
	int i;
	for (i = 0; i < MAX_CLIENTS; ++i) {
		if (client[i] == -1) {
			return i;
		}
	}
	return -1;
}

		for(index = 0; index < MAX_CLIENTS; ++index)
		{
			if (fds_clients[index] != -1)
			{
				FD_SET(fds_clients[index], &descriptors_set);
			}
		}

		/* computing max-fd */
		fd_max = max_of(fd_listen_socket, fds_clients);

		/* preparing timeout value */
		tv.tv_sec = 1;
		tv.tv_usec = 1000;	/* 1 milli */
					
		n_fd_ready = select(fd_max + 1, &descriptors_set, NULL, NULL, &tv);
	    if (n_fd_ready < 0)
        {
            if(EINTR == errno && count_pending==1) 
  			{
  				    count_pending=0;
  					++counter;
  					errno=0;
  					continue;
  			}
  			else
  			{ 
               error ("accepting ");
  			}
        }
		/* listening socket is readable */
		if (FD_ISSET(fd_listen_socket, &descriptors_set))
		{
			client_address_length = sizeof(client_address);
			
			//shoshan do we need here to check EINTR from timer? or select protection if good enough?
			fd_new_connection = accept(fd_listen_socket, (struct sockaddr*)&client_address, &client_address_length);
       
			index = find_free_cell(fds_clients);

			/* Too many clients */
			if (index == -1)
			{
					close(fd_new_connection);
			}
			else
			{
					fds_clients[index] = fd_new_connection;
			}
			--n_fd_ready;
		}
	
			/* check clients for data */
		index = 0;
		while ((index < MAX_CLIENTS) && (n_fd_ready > 0))
		{
			fd_socket = fds_clients[index];

				/* if this is a valid socket and it has something to read from */
			if ((fd_socket != -1) && (FD_ISSET(fd_socket, &descriptors_set))) 
			{
				--n_fd_ready;

				number_of_bits = read(fd_socket, buf , sizeof(buf));
				if (number_of_bits == 0) { /* connection closed by client */
					close(fd_socket);
					fds_clients[index] = -1;
				} else {
					  switch (*buf)
					  {
					  /* Parse the message from client and deal with it */
					 
					  case 'q':
					   ++msg_counter;
					   snprintf (msg, sizeof (msg), "msg %d-> Server quits.",msg_counter);
					   fprintf (logfile, "Server terminated on user request\n");
					   quit = 1;
					   break;
					  
					  case 'c':
					   ++msg_counter;
					   snprintf (msg, sizeof (msg), "msg %d->counter=%d",msg_counter,counter);
					   fprintf (logfile, "Re-starting life.\n");
					   break;

					  case 'r':
					   counter-=1000;
					   snprintf (msg, sizeof (msg), "msg %d->counter=%d",msg_counter,counter);
					   break;
					  default:
					  ++msg_counter;
					   snprintf (msg, sizeof (msg), "msg %d->Unknown command",msg_counter);
					   break;
					  }
						/* Ideally, this write should be made only after
						 * we made sure (using select) that the socket is
						 * ready for writing */
						/* Changing the code is not as simple as using select
						 * here. The right way to do it is have a queue of
						 * network IO operation. Whenever an operation is
						 * possible - perform it */
					//printf("sending line: %s\n", line);
					write(fd_socket, msg, strlen(msg));
				}
			}
			++index;
		}
	}
  return NULL;
 }
 
 
/* Do this when exiting */
void
close_server (void)
{
 
  /* Close log connection. */
  fclose(logfile);
  
  /* Remove our socket */
  unlink (SOCKETNAME);
  return;
}
 
/* This is an signal handler that is called when certain signals
are sent to us. We just set a flag. Doing anything else is
dangerous because of the signal async. nature.
 
Specifically, calling fprintf here is a mistake that can
cause very obscure and hard to find bugs
*/
   
void
signal_handler (int sig)
{
	switch (sig)
	{
	 case SIGALRM :          
	      count_pending =1;  /* marking count */
          break;
     case SIGTERM:
          term_pending = 1;
          break;
      case SIGINT:
          term_pending = 1;
          break;
      default:
        break;
      
	}
  return;
 
}
 

/* On error send message to log and quit (dtor will be called) */
void
error (char *msg)
{
  char buf[256];
  fprintf (logfile, "%s: %s", msg, strerror_r (errno, buf, sizeof (buf)));
  exit (1);
}
