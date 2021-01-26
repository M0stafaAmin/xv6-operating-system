#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{

	int parent_pid = getpid();
	int tickets = atoi(argv[1]);
	settickets(tickets);

	
	if(fork() == 0)
	{
		int child_pid = getpid();
		struct pstat st;
		getpinfo(&st);

		int parent_tickets = -1, child_tickets = -1;
        
        	int i;
		for (i = 0; i < sizeof(st.pid)/sizeof(st.pid[0]); i++)
		{
			if(st.pid[i] ==parent_pid)
				parent_tickets=st.tickets[i];
			
			else if(st.pid[i] ==child_pid)
				child_tickets=st.tickets[i];
		}
		
		printf(1, "parent: %d, child: %d\n", parent_tickets, child_tickets);
    		
    		exit();
	}
	
  	while (wait() > 0) ;
  
	exit();
}
