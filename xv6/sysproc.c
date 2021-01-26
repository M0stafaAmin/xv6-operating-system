#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"


int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int
sys_settickets(void) {
  int n;
  if(argint(0, &n) < 0)
      return -1;
 
  return settickets(n);
}

int
sys_getpinfo(void)
{
    struct pstat *d;
	if (argptr(0, (char **)&d, sizeof(struct pstat)) < 0)
	return -1;

	return getpinfo(d);
}

int
sys_mprotect(void){
  int d;
  int n = 0;
  if(argint(0, &d)<0 || argint(1, &n)<0)
    return -1;
  int rt=mprotect((void *)d,n);
  if(rt==-1)
  	exit();
  	
  return rt;

}

int
sys_munprotect(void){
  int d;
  int n = 0;
  if(argint(0, &d)<0 || argint(1, &n)<0)
    return -1;
  
  int rt=munprotect((void *)d,n);
  if(rt==-1)
  	exit();
  	
  return rt;

}

int
sys_clone(void) {
  void *fcn, *arg1, *arg2, *stack;
  //check if arguments is valid before calling clone syscall
  if (argptr(0, (void *)&fcn, sizeof(void *)) < 0)	return -1;
  if (argptr(1, (void *)&arg1, sizeof(void *)) < 0)	return -1;
  if (argptr(2, (void *)&arg2, sizeof(void *)) < 0)	return -1;
  if (argptr(3, (void *)&stack, sizeof(void *)) < 0)	return -1;
	
  return clone(fcn, arg1, arg2, stack);
}

int
sys_join(void)
{
  void **stack;
  int stackArg;
  stackArg = argint(0, &stackArg);
  stack = (void**) stackArg;
  return join(stack);
}

int sys_initlock_t(void)
{
  struct ticketlock *tl;
  if (argptr(0, (char**)&tl, sizeof(struct ticketlock*)) < 0) return -1;
  
  initlock_t(tl);
  return 0;
}

int sys_acquire_t(void)
{
  struct ticketlock *tl;
  if (argptr(0, (char**)&tl, sizeof(struct ticketlock*)) < 0) return -1;
  
  acquire_t(tl);
  return 0;
}

int sys_release_t(void)
{
  struct ticketlock *tl;
  if (argptr(0, (char**)&tl, sizeof(struct ticketlock*)) < 0) return -1;
  
  release_t(tl);
  return 0;
}


