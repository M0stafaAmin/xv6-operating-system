#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "fcntl.h"


int
main(int argc, char *argv[]){

	settickets(10); //change intial value of parent
 	
 	const int size=argc-1;
	int child_pid[size];
 	int numtickets[size];

		
	for(int i=0 ;i<size;i++){
		numtickets[i]=atoi(argv[i+1]);
	}	
	
	for(int i=0;i<size;i++)
	{
		child_pid[i]=fork();

		if(child_pid[i]==0){
			
			settickets(numtickets[i]);
			for (;;);
		}

	}
    
	struct pstat st;

	while(1)
	{
	
		getpinfo(&st);
		printf(1, "\nPID\t|\tUSED?\t|\tTickets\t|\tTicks\n");

		for(int i=0;i<size;i++)
		{
			for (int j = 0; j < sizeof(st.pid)/sizeof(st.pid[0]); j++)
			{
				if(st.pid[j] ==child_pid[i])
					printf(1, "%d\t|\t%d\t|\t%d\t|\t%d\n", st.pid[j], st.inuse[j], st.tickets[j], st.ticks[j]);
			}
		}
		
		sleep(200);
    	}

	exit();
}
