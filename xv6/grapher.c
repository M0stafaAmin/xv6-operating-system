#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "fcntl.h"


int
main(int argc, char *argv[]){
 	int numtickets[]={10,20,30};
	int child_pid[3];

	settickets(10);//change intial value of parent
	
	int i;
	for(i=0;i<3;i++){
		child_pid[i]=fork();

		if(child_pid[i]==0){
			
			settickets(numtickets[i]);
		
			for (;;);
		}

	}
    
	struct pstat st;
	int time=50;

	printf(1,"\nProcess A (%d tickets)\tProcess B (%d tickets)\tProcess C (%d tickets)\n",numtickets[0],numtickets[1],numtickets[2]);
	
	while(time--){
		getpinfo(&st);
		
		for(i=0;i<3;i++){
			for (int j = 0; j < sizeof(st.pid)/sizeof(st.pid[0]); j++)
			{
				if(st.pid[j] ==child_pid[i])
					printf(1, "%d\t",st.ticks[j]);
			}
		}
		printf(1,"\n");
		sleep(200);
    }

	exit();

}

