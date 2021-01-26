#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

int
main(int argc, char *argv[])
{
  
  char *protectedValue = sbrk(PGSIZE);
  *protectedValue=100;
  printf(1, "\nHere parent ,we protect whole page that our value in \n"); 
  mprotect(protectedValue, 1) ;
  
  int child=fork();
  if(child==0){
	printf(1, "protected value = %d\n",*protectedValue);
		printf(1, "\nHere in child ,we try to change value after unprotecting page\n");  
        munprotect(protectedValue, 1) ;  
        *protectedValue=5;
        printf(1, "After unprotecting the value became = %d\n",*protectedValue); 
        exit();
  }
  
  else if(child>0){
        wait();
        printf(1,"\nHere in parent ,we try to change value without unprotecting page\n");
        printf(1, "System should trap due to try to write in protected page\n"); 
        *protectedValue=5; 
        printf(1, "\nTest Failed\n");
        exit(); 
  } 
  
 exit();
}
