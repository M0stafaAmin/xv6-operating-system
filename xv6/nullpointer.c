#include "types.h"
#include "stat.h"
#include "user.h"

#define NULL ( ( void *) 0)

void 
nulltest(){

    int *p = NULL;
    printf(1, "This is a test for NULL pointer deference \n");    
    printf(1, "System should trap due to NULL pointer deference\n"); 
    
    printf(1, "%x %x\n", p, *p);
    printf(1,"\nThis statement will not be printed\n");
    printf(1,"\nNow, System recognize null pointer\n");

}

void 
fork_nulltest(){
	
	int parent_pid = getpid();
	
	int rc=fork();
	if (rc == 0) {
	  int * p = NULL;
	  printf(1, "This is a test for fork test \n");
	  printf(1, "System should trap due to NULL pointer deference\n"); 
	  
	  printf(1, "%x %x\n", p, *p);
	  printf(1, "\nThis statement will not be printed\n");
	  printf(1,"\nNow, System recognize null pointer\n");

	  kill(parent_pid);
	  exit();
	} 
	else {
	  wait();
	}
	
	printf(1, "TEST PASSED\n");
}


int
main (int argc ,char *argv[])       
{
	char *option=argv[1];	
		
	if(strcmp(option,"test")==0)  	
		nulltest();
		
	else if(strcmp(option ,"fork-test")==0)
		fork_nulltest();
	
    exit();
}

