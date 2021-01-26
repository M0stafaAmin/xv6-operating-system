#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

void
length()
{
  
	char *protectedValue = sbrk(PGSIZE);
	*protectedValue=100;
	mprotect(protectedValue, 2) ;
	
	*protectedValue=5; 
	printf(1, "\nTest Failed\n");
  
}

void
aligned()
{
  
	char *protectedValue = sbrk(2*PGSIZE);
	*protectedValue=100; 
	mprotect(protectedValue+1 , 1 ) ;
	
	*protectedValue=5; 
	printf(1, "\nTest Failed\n");

}

int
main (int argc ,char *argv[])       
{
	char *option=argv[1];	
		
	if(strcmp(option,"length")==0)  	
		length();
		
	else if(strcmp(option ,"aligned")==0)
		aligned();
	
    exit();
}

