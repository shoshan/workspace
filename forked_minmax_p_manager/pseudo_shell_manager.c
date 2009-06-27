//TODO check non interactives jobs 
//***************************************************************************************
//  File: manager.c
//
//  Description: managing forking childs and childs exit signals 
//
//  Written: shoshan dagany , shoshan dagany@gmail.com
//***************************************************************************************
//#define _DEBUG
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
typedef int bool;
#define true 1
#define false 0


// pid node - holding pid data.
struct pid_node;
struct pid_node{
       struct pid_node* next;  // pointer to next in the list. 
 	   unsigned  pid_num;     
	   unsigned  job_num;      //job num in the runing serias 
	   char*     command_line; // the command line of the pid
};
/////////////////////////////////////////////////////////////////////////
//declaration of the funtions to keep it simple.
void pid_add_first(struct pid_node* head, struct pid_node* node);
struct pid_node* pid_remove_first(struct pid_node* head);
struct pid_node*  pid_search(struct pid_node* head, unsigned pid);
struct pid_node* pid_extract (struct pid_node* head, unsigned pid);
void  pid_insert(struct pid_node* head, unsigned pid, unsigned job,char *command_line);
struct pid_node*  pid_iterate(struct pid_node* head);
void job_new(char *job_detail);
void job_ended(int signo);
void jobs();
void manager_loop();
void parse(char *line,char**  command,char*** parameters,char **original_line );
//////////////////////////////////////////////////////////////////////////////////
static struct pid_node  runing_process_list={0,0,0}; // head pid pointer , ( data fields are  not used containing 0)
//just the pointer is used here.

static struct pid_node  free_list={0,0,0}; // head pid field is not used containing 0
//its holding the free list of pid structs , used to save malloc and free cpu time

//pointer to the tail of the processes list 
//its for saving time in insertions o(1) insted of o(n)
static struct pid_node* tail=&runing_process_list;
/////////////////////////////////////////////////////
//handler called by SIGCHILD signal.
// used to delete ended pid from the list.
void job_ended(int signo){
    int status;
	int pid=0;
	struct pid_node* process=NULL;
        if (signo != SIGCHLD){
	  exit(-1);
        }
    
    //get pid of finished process
    pid=waitpid(-1,&status,WNOHANG);
	process=pid_extract(&runing_process_list, pid);
	if (!process){
#ifdef _DEBUG
	   printf("error:pid not found %d\n",pid);
#endif	  
 return;
	}
	memset(process,sizeof(struct pid_node),0);
	
	//adding ended process pid struct to free list.
	pid_add_first(&free_list,process);
}
/////////////////////////////////////////////////////
//pushing to the begining of the list.
void pid_add_first(struct pid_node* head, struct pid_node* node){
	node->next=head->next;
	head->next=node;
}
/////////////////////////////////////////////////////////////////
//removing from the begining of the list.
struct pid_node* pid_remove_first(struct pid_node* head){
	struct pid_node* node=head->next;
	if (node){
	   head->next=node->next;
	   node->next=NULL;
	}else {
	  return NULL;
        }
	if (node->command_line){
	  free (node->command_line);
	  node->command_line=NULL;
	}
	return node;
}
/////////////////////////////////////////////////////////////////
//searching in a list for specific pid node.
struct pid_node*  pid_search(struct pid_node* head, unsigned pid){
   	struct pid_node* previous=head;
	while ( previous->next != NULL){
		if (previous->next->pid_num==pid){
		    return previous;
		}
		previous=previous->next;
	}
	return NULL;
}
//////////////////////////////////////////////////////////
//extracting specific pid node from the list - head parameter is the head of the list.
struct pid_node* pid_extract (struct pid_node* head, unsigned pid){
	struct pid_node* previous=NULL;
	struct pid_node* found=NULL;
	previous=pid_search( head,  pid);
	if ( previous !=NULL){
	        found=previous->next;
		if (found==tail){
	           tail=previous;
                }
                previous->next=found->next;
#ifdef _DEBUG
        printf("pid removed%d \n",pid);
#endif
	}
	return found;
}
/////////////////////////////////////////////////////////////
//inserting new pid data to the process list.
void  pid_insert(struct pid_node* head, unsigned pid, unsigned job,char *command_line){
	
	struct pid_node* tmp=pid_remove_first(&free_list);
	if (!tmp){
		tmp=(struct pid_node*) malloc(sizeof (struct pid_node));
	}
	if (!tmp){
	  printf("cannot insert new pid - not enough memory\n");
		return;
	}
	tmp->next=NULL;
	tmp->pid_num=pid;
	tmp->job_num=job;
	tmp->command_line=command_line;
	tail->next=tmp;
    tail=tmp;
	
#ifdef _DEBUG
        printf("pid inserted %d\n",pid);
#endif
}
/////////////////////////////////////////////////////
//iterating list.
//if head is not null then the iterator is initialized
//if head is null the the iterator continues from previously initilized call.
struct pid_node*  pid_iterate(struct pid_node* head){
   	static struct pid_node* node=NULL;
	if (head!=NULL){
	   node=head;
	}
	if (node!=NULL){
	    node=node->next;
	}
	return node;
}
/////////////////////////////////////////////////////
//job_new - creating new job according to input line from the user.
void job_new(char *job_detail){
     static unsigned job_index=0;
	 int pid=0;
	 char*  command=NULL;
     char** parameters=NULL;
     char *original_line=NULL;
         (job_detail)[strlen(job_detail)-1]=0;
     if (strlen(job_detail)==0){   // its a meaningles input line
	   return ;
         }

	 //parsing the input line- job details
	 //writing to command , parameters , and original line
	 parse(job_detail,&command,&parameters,&original_line);

	 //if the input lime is jobs then no new process if created , the list is printed instead.
	  if (strcmp(command,"jobs")==0){
	    jobs();
             free(command);
             free(original_line);
		 return;
	  }

// forking new process
     pid=fork();
	 if (pid<0){
	   printf("cant create job\n");
	   return;
	 }
	 if (pid==0){
       //setting child process to do what the user wanted 
	   if  (execvp(command,parameters)==-1){
	   
             #ifdef _DEBUG
	        int j=0; 
                printf("error in execvp  %s ",command);
                while (parameters[j]){
		          printf(" %s",parameters[j]);
                  j++;
                }
                printf("\n");
           #endif  
		
                }
		exit(-1);
	 }else {
			 if (pid>0){
				 //inserting new pid to the processes list.
				 int j=0;
					 pid_insert(&runing_process_list,pid,job_index,original_line);
       				 job_index++;
					 
					 //free  memory
					 if (parameters){
					   while (parameters[j]){
						 free (parameters[j]);
						 j++;
					   }
					 }
					 free(command);
		           
						#ifdef _DEBUG
						 printf("reached parent\n");
						#endif  
				 return;
			 }
	 }
}
/////////////////////////////////////////////////////
//jobs - iterate the processes list and printing information about the jobs
void jobs(){
struct pid_node* process=NULL;
#ifdef _DEBUG
        printf("reached jobs \n");
#endif
   process=pid_iterate(&runing_process_list);
   while (process ){
	   printf("%d\t%d\t%s\n",process->job_num,process->pid_num,process->command_line);
	   process=pid_iterate(NULL);  
 }
}
////////////////////////////////////////////////////////////
//manager_loop - endless loop to read user input and executing programs
void manager_loop(){
	while(true){
	  static char command[300]={0};
      fprintf(stderr,"\nprompt>"); 
      fgets(command,290,stdin);
      job_new(command);
	}
}
////////////////////////////////////////////////////
//parse input line and creating command and parameters for exec.
void parse(char *line,char**  command,char*** parameters,char **original_line ){
  int length=strlen(line);
  char* seps=" \t";
  char* token =strtok(line,seps);
  *parameters=(char**)malloc(length/2*sizeof(char**));
  (*original_line)=malloc(length+1);
  strcpy(*original_line,line);
    
 if (token==NULL){
    (*command)=malloc(length+1);
    strcpy(*command,line);
    (*parameters)[0]=(char*)malloc(length+1);
    strcpy((*parameters)[0],line);
    (*parameters)[1]=NULL;
  }else
  {
   int counter;
  (*command)=malloc(length+1);
   strcpy(*command,token);
   (*parameters)[0]=(char*)malloc(length+1);
   strcpy((*parameters[0]),line);
   counter=1; 
   token=strtok(NULL,seps);
   while(token){
         (*parameters)[counter]=(char*)malloc(strlen(token));
          strcpy((*parameters)[counter],token);
          counter++;
          token=strtok(NULL,seps);
  }
	 (*parameters)[counter]=NULL;
  }
}
/////////////////////////////////////////////////////
// main , making SIGCHILD handler and calling manager_loop
int  main(){
  signal(SIGCHLD,job_ended);
  manager_loop();
  return (0);
}
