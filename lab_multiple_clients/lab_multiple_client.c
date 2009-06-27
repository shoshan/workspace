//lab_multiple_client.c client to run in loop with random commands or
// one command end exit , gets command from program parameters
//AUTHOR shoshan.dagany@gmail.com 
//REFERENCES embedded linux course gby--github.com
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h> 
#include <fcntl.h>
#define SOCKETNAME "/tmp/lab_socket"
#define MSGLEN (100)
void error (char *);
 
int
main (int argc, char *argv[])
{
  /* Socket stuff and buffers */
  int sockfd, servlen, bytes_number;
  struct sockaddr_un serv_addr;
  char buffer[MSGLEN];
  char socketpath[256];
  char *command=NULL;
  bool connection_valid=false;
  bool repeat_commands=false;
  char commands[2][3];
  strcpy(commands[0],"c" );
  strcpy(commands[1],"r");
  int fd_rand=open("/dev/urandom",O_RDONLY);
  char rand_num;
  if (fd_rand<0)
  {
  	printf("failed open rand\n");
  	exit(-1);
  }
  if (argc != 2)
    {
      printf ("Usage: %s [crq] \nc-Send back counter.\nr-reset counter\nq-terminate server\n", argv[0]);
      repeat_commands=true;
    }
    else
    {
    	command=argv[1];
    } 
 
   bzero(buffer, sizeof(buffer));
 
  /* Build our socket */
  snprintf(socketpath, sizeof(socketpath), "%s", SOCKETNAME );
 
 /* connect to socket */
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy (serv_addr.sun_path, socketpath);
  servlen = strlen (serv_addr.sun_path) + sizeof (serv_addr.sun_family);
 
  if ((sockfd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
    error ("Creating socket");
 
  while ( connect (sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
  {
    printf("Connecting: No such file or Directory\n");
    printf("Connecting: retry in 3 seconds\n");
    sleep(3);
  }
  connection_valid=true;/* a simple console client for use from the shell */
  
  do 
  {
	   /* Send what we have to say and print reply to terminal */
	   if (repeat_commands)
	   {
	   	  bytes_number=read(fd_rand, &rand_num,1);
	   	  if ((unsigned)rand_num<20  )
	   	  {
	   	  	command=&commands[1][0];
	   	  }
	   	  else 
	   	  {
	   	  	command=&commands[0][0];
	   	  }
	   }
	   bytes_number=write (sockfd, command, strlen(command));
	   if (bytes_number<=0)
	   {
	   	  printf("error writing\n");
	   }
	   if (connection_valid)
	   {
	     bytes_number = read (sockfd, buffer, MSGLEN);
	     if (bytes_number<=0)
	     {
	   	    printf("error read\n");
	   	    connection_valid=false;
	     }
	     else
	     {
	     	printf ("%s#\n%s\n", command, buffer);
	     } 
	   }
	   sleep(1);
  }while (connection_valid && repeat_commands);
  printf("connection ended\n");
 
  return 0;
}
 
void
error (char *msg)
{
  perror (msg);
  exit (0);
}