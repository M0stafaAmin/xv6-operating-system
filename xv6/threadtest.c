#include "types.h"
#include "stat.h"
#include "user.h"
#include "ticketlock.h"

#define NULL (void *)(0)
struct ticketlock lock;

int sharedVariable =0;

void increment(void *str, void *iteration)
{	
	int pid = getpid();
	
	lock_acquire(&lock);
	printf(1, "** Thread with PID: %d %s **\n",pid, str);
	lock_release(&lock);
	
	for(int i = 1 ; i <= (int) iteration ; i++)
	{	
		lock_acquire(&lock);
    		printf(1, "(%d) >> %d\n", pid, i);
    		sharedVariable++;
    		lock_release(&lock);
	}
	
	lock_acquire(&lock);
	printf(1, "*** (%d) Finished ***\n", pid);
	lock_release(&lock);
	
	exit();
}


void test_one_thread()
{	
	lock_init(&lock);
	printf(1, "*** Testing single thread ***\n");
	
	thread_create(&increment, (void*) "(Single Thread)" , (void*)30);
  	thread_join();
}



void test_multi_threads(int num)
{
	lock_init(&lock);
	printf(1, "\n*** Testing multi threads ***\n");
	
	for(int i = 1 ; i <= num ; i++)
		thread_create(&increment, (void*) "started", (void*)50);

  
  	for(int i = 1 ; i <= num ; i++)
		thread_join();
}



int
main(int argc, char *argv[])
{
  	const int NTHREADS = 4;
  	
  	test_one_thread();
  
  	test_multi_threads(NTHREADS);
  	
	printf(1, "*** Shared Variable = %d ***\n",sharedVariable);

  	exit();
}
