//lab2_threads.c create process spawn threads , after threads fininshed displaying log 
//AUTHOR shoshan.dagany@gmail.com
//REFERENCES embedded linux course gby--github.com 
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>


#include <errno.h>
#include <string.h>
#define  THEADS_NUMBER 5
#define  TRACE_SIZE  8000 
  	 int fd=-1;
  typedef struct tracer_{
     unsigned int        total_index;
     unsigned int        thread_index;
     unsigned int        index;
     unsigned int        number;
  }tracer;
  tracer trace[TRACE_SIZE+10]; 
  int trace_size=0;
  int trace_size_msb=0;
  
  unsigned char *ptrace=(unsigned char*)&trace[0];
  ////////////////////////////////////////////////////////////////////////////////
  void* thread_function(void *arg)
  {
  	    int it=0;
    	int return_code=0; 
    	int random_number;
    	//printf("i'm thread #%d\n", (int)arg);
        int local_index;
        int thread_index=((int) arg) &0x7;
        int loop_size= (int ) arg;
        for (it=0;it<loop_size;it++)
        {
        	if (trace_size>TRACE_SIZE)
        	{ 
        		trace_size_msb+=TRACE_SIZE; 
        		trace_size=0;
        	}
        	local_index=trace_size;
    		trace_size++;
    		if (read(fd,&random_number,1)==1)
    		{
    			trace[local_index].total_index=trace_size+trace_size_msb;
    			trace[local_index].thread_index=(int) thread_index;
    			trace[local_index].index=it;
    			trace[local_index].number=random_number;
    			//printf("thread #%d[%d]  %x\n", (int)arg,it, random_number);
    		}
    		else 
    		{
    			printf("thread #%d[%d] read  failed \n", (int)arg,it);
    		}
        }
   printf("thread #%d[%d] trace_size=%d  finished \n", (int)arg,it,trace_size);
   sleep(1);
   pthread_exit( (void*)NULL); 
  }
//////////////////////////////////////////////////////////////////////////////////  
int main (int argc , char **argv)
{
  int param=512;
  pid_t pid;
  if (argc!=1)
  {
    printf("Usage: Lab2 <Number>  ");
    return -1; 
  }
  //param=atoi(argv[1]);
  int i;
  if (pid=fork())
  {
    int status;
    wait (&status);
    if (WIFEXITED(status))
    {  
       printf("goodbye\n");
       
    }
       
  }
  else 
  {
    	fd=open("/dev/random",O_RDONLY);
    	if (fd<0)
    	{
    	 printf("open dev random failed\n");	
    	 return(-1);
       	 
    	}
    	memset(trace,0xa5,sizeof(trace));
    printf("trace=%x\n",(unsigned int)trace);	
     int ii;
     void*  status;
     pthread_t threads[5];
     for (ii=0;ii<5;ii++)
     {
       int rc=pthread_create(&threads[ii],NULL,&thread_function,(void*)(ii+param) );
       if (rc)
       {
       	 printf ("createerr");
       	 printf ("create thread #%d returned error %d\n",ii,rc);
       	 printf ("%s\n",strerror(errno));
       	 return;
       }
     }
     for (ii=0;ii<5;ii++)
     {
       printf("trace=%x\n",(unsigned int) &trace[0]);	
       printf("now joining thread  %d\n",ii);	
       pthread_join(threads[ii],&status);
     }
       printf("trace_size =%d\n",trace_size);
       for (ii=0;ii<trace_size;ii++)
       {
       	   printf( "i=%3d ti=%3d tli=%3d n=%3d \n",ii,trace[ii].thread_index,trace[ii].index,trace[ii].number );
       }
  
     printf("got parameter %d\n",param);
  }
}  
