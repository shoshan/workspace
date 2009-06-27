//TODO also check without debug defines
#define DEBUG_OUTPUT
#define DEBUG_INPUT 
//***************************************************************************************
//  File: minmax.c
//
//  Description: finding min max 4 bit number using 8 bit of exit status from forked processes working on half input
//
//  Written: shoshan dagany , shoshan dagany@gmail.com
//***************************************************************************************
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

#define MAX_NUM 32  // maximum # of numbers to find min max 
typedef int bool;
#define true 1
#define false 0

/////////////////////////////////////////////////////////////////////////
//make_parameters make ARGV parameters for the next call to find minmax.
//by copying part of the previous numbers to the new parametrs.
//it also copy the name of the program from ARGV[0] to new ARGV[1] 
//this indicate the new run is not the first run in the tree
void make_parameters (char **new_parameters,char** parameters,int offset_first_data,int start,int end){
  int i;
  new_parameters[0]=parameters[0];
  new_parameters[1]=parameters[0];
  for (i=0;i<end-start+1;i++){
     new_parameters[i+2]=parameters[offset_first_data+start+i];
  }
  new_parameters[i+2]=NULL;

}
//////////////////////////////////////////////////////////////
//parse_status- parsing the status returned to find out the max location and the min location
//status - input parameter
//*max will hold the max location
//*min will hold the min location
void parse_status(int status , int* max ,int* min){
         
  //extracting 8 bit - returned from the child process
  int  locations=WEXITSTATUS(status);        
   
     *max=locations&0x0f;
	 *min=(locations&0xf0)>>4;
         #ifdef DEBUG_OUTPUT
         printf("parse %x , %d,%d\n",status,*max,*min);
         #endif
         return;
        }
    
//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// main gets argc - up to 32 numbers stored in argv.
// returns a print of the min & max numbers- if its the root process.
// returns - the location of min and max in the serias if it is a child process 
int main(int argc, char *argv[])
{
  extern int errno;
  int numbers[MAX_NUM];    //array of lines size of system maximum 
  int size = 0;            //total numbers to see min max 
  bool is_root=true;       //root is the first call / child processes are not root.
  int  first_num_offset=1; // first num offset in argv
  int  minimum=0;          //hold minimum number
  int  maximum=0;          //hold maximum number 

  int  i=0;                //index
  int  paralel_counter=0;  //counts how many times we splitted the input serias - should come to 1 split for two parts. 
  int pid;                 //pid returned by fork.
  int j;         
  int last_pid[2];         //holds the 2 pid's - one for first half serias , other to second half serias
  int minimum_location[2]={0,0};   //holds minimum number location in first and second halves.
  int maximum_location[2]={0,0};   //holds maximum number location in first and second halves.
  int min_location=0,max_location=0; //holds locations , eventually its the min and max locations in the complete serias.
  int min[2];                        //holds min numbers in first , second halves.
  int max[2];                        //holds max numbers in first , second halves.  

  if(argc < 2){                      // checking for at least one number in the input
    fprintf(stderr,"USAGE: %s num1 num2 num3 ...num32 \n", argv[0]);
     exit(1);
  }
  
  //checking the the first parameter is the program name - indication of root.
  if (  strcmp(argv[1],"forked_minmax")==0 || strcmp(argv[1],"./forked_minmax")==0) {
    is_root=false;  
  }  
  
#ifdef DEBUG_INPUT 
    printf("got parameters " );
    for (j=0;j<argc;j++){
      printf("%s ",argv[j]);
    }
 printf(" is root= %d\n ",is_root);
#endif
    
 //cheking for maximum input length
  if (is_root && argc>33){    
     printf("USAGE: %s num1 num2 num3 ...num32 \n", argv[0]);
     exit(1);
  }

  //finding out the first number offset and the size of the input serias.
  if (!is_root ){
    first_num_offset=2;
    size=argc-2;

  }  else {
    first_num_offset=1;
    size=argc-1;
  }
  
  //if input serias is larger then 2 the we split the serias to 2 and calculate min max for first and second half by recursivly executing.
  if (size>=3){
    char *parameters1[32];
    char *parameters2[32];
    char **new_parameters[2]={parameters1,parameters2};
    int half=size/2;     //half is calculated by size
    #ifdef DEBUG_OUTPUT
       printf("size=%d  half=%d \n",size,half);     
    #endif
    make_parameters ( new_parameters[0], argv, first_num_offset,0,half-1);
    make_parameters ( new_parameters[1], argv, first_num_offset,half,size);
    
	//loop for forking the child processes 
	//and holding the pid of the child processes for future wait command
	while (paralel_counter<2){
    int pid;
    paralel_counter++;
    if ((pid = fork()) < 0){ // spawn child process failed 
    perror("Fork failed\n");
    exit(1);
    }

  if (pid == 0){  // child code 
    #ifdef DEBUG_INPUT   
     printf("call exec counter=%d,%s,%s\n",paralel_counter-1,new_parameters[paralel_counter-1][0],new_parameters[paralel_counter-1][1]);
    #endif  
    
	 //executing the min max program with its half of the serias
	 execvp(argv[0],new_parameters[paralel_counter-1]);
    exit (-1);
  }
  else{ // parent code
    last_pid[paralel_counter-1]=pid; 
    continue;//reach here after forking all children processes 
  }
}

//loop for collecting results
 for ( i=0;i<2;i++){ 
   int status;
    // waiting for  child process to end 
    pid=wait(&status);
    if (pid < 0){
      perror("Wait failed\n");
      exit(3);
    }
    
	//calculating locations from status.
	parse_status( status ,  &max_location , &min_location);
    if (pid==last_pid[0]){
      min[0]=atoi(argv[min_location+first_num_offset]); 
      max[0]=atoi(argv[max_location+first_num_offset]); 
      minimum_location[0]=min_location;
      maximum_location[0]=max_location;
      
    }
if (pid==last_pid[1]){
      min[1]=atoi(argv[min_location+first_num_offset+half]); 
      max[1]=atoi(argv[max_location+first_num_offset+half]); 
      minimum_location[1]=min_location;
      maximum_location[1]=max_location;
      
    }
    
 }

 //comparing min and max of the two halves to see which is  larger and smaller.
     if (min[1]<min[0]){
      min_location=minimum_location[1]+half;
#ifdef DEBUG_OUTPUT
      printf("min_location=%d minimum_location[1]=%d + half=%d =%d\n",min_location,minimum_location[1],half);
    #endif
     
     }
     else {
    #ifdef  DEBUG_OUTPUT
       min_location=minimum_location[0];
  printf("min_location=%d minimum_location[0]=%d +first_num_offset=%d\n",min_location,minimum_location[0],first_num_offset);
    #endif
          
}
  
    if (max[1]>max[0]){
      max_location=maximum_location[1]+half;
    }else {
      max_location=maximum_location[0];
    }

	// calculating minimum and maximum again from locations
    minimum=atoi(argv[min_location]);
    maximum=atoi(argv[max_location]); 
#ifdef DEBUG_OUTPUT
    printf("minimum=%d =atoi(argv[min_location=%d ]\n",minimum,min_location);   
#endif
  }

  //calculating min and max for serias of 1 or 2 numbers.
  //this size dont needs a split and recursive fork and its the end condition.
  if (size<3){
    for (i=0;i<size;i++){
      numbers[i]=atoi(argv[i+first_num_offset]);
    }
    min_location=0;
    max_location=0;
    minimum=numbers[0];
    maximum=numbers[0];
    
	//if its two numbers the updating min and max.
	if (size==2){
      if (minimum >numbers[1]){
	minimum=numbers[1];
        min_location=1;
      }else {
        maximum=numbers[1];
        max_location=1;
      }
    }
   
  }
#ifdef DEBUG_OUTPUT
    printf("got " );
    for (j=0;j<argc;j++){
      printf("%s ",argv[j]);
    }
  printf("is_root=%d,%s,%s,minimum =%d , maximum=%d\n",is_root,argv[1],argv[2],minimum,maximum);
#endif

  //for  CHILD processes returns the min and max location as bits in the exit number.
  if (!is_root){
    int returned_value=(0xf & max_location)+ ( (min_location<<4) &0xf0) ;
    #ifdef DEBUG_OUTPUT
    printf("max_loaction=%d min_location=%d\n",max_location,min_location );
    printf("returned_value=%x\n",returned_value);
    printf("-------------------\n");
    #endif
    exit(returned_value);
  }

  //for root print the min and max number as requested.
  printf("max=%d\nmin=%d\n",atoi(argv[max_location+first_num_offset]),atoi(argv[first_num_offset+min_location]));
#ifdef DEBUG_OUTPUT
  printf("max_loc=%d\nmin_loc=%d\n",first_num_offset+max_location,first_num_offset+min_location);
#endif
  exit(0); 
}



//***************************************************************************************
//                                    EOF
//***************************************************************************************
