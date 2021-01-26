# Table of contents
* [Lottery Scheduler](#lottery-scheduler)
* [Null-pointer Dereference](#null-pointer-dereference)
* [Read-only Code](#read-only-code)
* [Kernel Threads](#kernel-threads)
* [References](#references)
* [Team Members](#team-members)

------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lottery scheduler
* We will implementing lottery scheduler in xv6 
* The basic idea is simple : assign each running process a slice of the processor based in proportion to the number of tickets it has " the more tickets a process has, the more it runs " , algorithm draws a random ticket to select the next process


Specifically, in this project we are required to do the following:

1. we add new variables to hold number of tickets process has and number of ticks.
2. change schedular algorithm to work as lottery scheduler.
3. We have to define a new system call to set number nof tickets `settickets()`.
4. We have to define a new system cal  to get all processes information `getpinfo()`.
5. make sure a child process inherits the same number of tickets as its parents

## 1. Adding number of tickets , ticks

1. in proc.h : 
* we have to add `int tickets;` , `int ticks;` in proc struct.
2. in proc.c : 
* in function `allocproc()` add after found label
 ```
 found : 
     // code 
     p->tickets=1;
     p->ticks=0;
```
------------------------------------------------------------------------------------------------------------------------------------------------------------------
## 2. Modify scheduler algorithm
### all in proc.c 
#### 1. add  new function `tickets_sum(void)`:
* we will need this function in scheduler function
* this function calculate total number of tickets for all runnable processes
```
int
tickets_sum(void){
  struct proc *p;
  int ticketsTotal=0;

  //loop over process table and increment total tickets if a runnable process is found 
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if(p->state==RUNNABLE){
      ticketsTotal+=p->tickets;
    }
  }
  return ticketsTotal;  // returning total number of tickets for runnable processes
}
```
#### 2. Modify scheduler function
#####  prepare all needed variables `long total_tickets = 0 ,counter = 0 ,long winner = 0;`
and in the for loop :
* `total_tickets = tickets_sum();` equal to return value of function 
* ` winner = random_at_most(total_tickets);` winner equal randome variable generated by this function [0:total_tickets]  
	1. and for this point we have to `#include "rand.h"` in proc.c
	2. and add in makefile 
	```
	OBJS = \
		rand.o\
	```
* we will count the total number of tickets for each runnable process , when the counter becomes greater than the rondom number
this process will run
```
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
	     //change start
      counter += p->tickets; 

      if (counter < winner) { 
            continue;
      }
      p->ticks += 1; //increment number of times process work
      //change end
      
      //rest of the code
```
------------------------------------------------------------------------------------------------------------------------------------------------------------------
## 3. Implementing `setticket()` 

1. We will add a wrapper function in `sysproc.c` to make sure the user has provided the right number and type of arguments before forwarding the arguments to the actual system call.
```
int
sys_settickets(void) {
  int n;
  if(argint(0, &n) < 0)
      return -1;
 
  return settickets(n);
}
```
2. Now we will implement the `settickets()` system call in `proc.c`
```
int 
settickets(int tickets)
{

  if(tickets < 1)
    return -1;
    
  struct proc *proc = myproc();
  
  acquire(&ptable.lock);
  ptable.proc[proc-ptable.proc].tickets = tickets;
  release(&ptable.lock);
  
  return 0;
}
```
3. Now that we have the implementation of the set tickets system call, we need to glue it in to the rest of the operating system. To do this we will need to make changes to the following five files:

* `syscall.h`: Add a new system call number to the end of the #define list. 

  `#define SYS_settickets 22`

* `syscall.c`: Add an extern definition for the new system call and add an entry to the table of system calls.

  ```
  extern int sys_settickets(void);
  
  [SYS_settickets] sys_settickets,
  ```

* `usys.S`: Add an entry for clone.

  `SYSCALL(settickets)`
  
* `user.h`: Add a function prototype for the system call.

  `int settickets(int);`

* `defs.h`: Add a function prototype for the system call.

  `int  settickets(int);`
------------------------------------------------------------------------------------------------------------------------------------------

# Null-pointer Dereference

In XV6 if you dereference a null pointer such like that:
```c
  int *p = NULL;   
  printf(1, "%x %x\n", p, *p);
```
You will not see page fault exception but you will find opcode exception in some versions of XV6 or you will see whatever code is the first bit of code in the program that is running.

![Screenshot from 2021-01-25 12-48-15](https://user-images.githubusercontent.com/47724391/105696209-ac9a3b80-5f0b-11eb-8e71-689b41bd64c7.png)

Trap number 6 meaning an illegal opcode , see `traps.h`

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

When the user trying to dereference a null pointer xv6 go to virual address zero (first page at page table of the process) and access memory using it.

## To fix this problem:
### when null dereference occurs XV6 mustn't find virual address zero (first page at page table of the process) by:

#### 1. Making XV6 load the program into the memory not from the address 0 but from the next page which is in fact address 4096 that is 0x1000.

##### In `Makefile`

replace line
```makefile
$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
```
with
```makefile
$(LD) $(LDFLAGS) -N -e main -Ttext 0x1000  -o $@ $^
```

#### 2. Making process itself to be loaded starting from address 4096 (`page number 2`).

##### In `exec.c`
modify line 	`sz = 0`  to  `sz =PGSIZE` .

#### 3. Making modification to not transfering first page from parent to child while using `fork()`. 
##### In `vm.c` inside `pde_t* copyuvm(pde_t *pgdir, uint sz)`

modify line 	`for(i = 0; i < sz; i += PGSIZE)`  to  `for(i = PGSIZE; i < sz; i += PGSIZE)` .

#### 4. Understanding `traps.h` to know needed types of traps, focused on `T_PGFLT --> page fault`.

#### 5. Adding some code at `trap.c` inside `void tvinit(void)` required to Initializes the IDT (Interrupt Descriptor Table) table.
```c
case T_PGFLT :
    cprintf("pid %d %s: trap %d err %d on cpu %d "
    "eip 0x%x addr 0x%x--kill proc\n",
     myproc()->pid, myproc()->name, tf->trapno,
     tf->err, cpuid(), tf->eip, rcr2());
     cprintf("This trap cause null pointer execption\n");
      myproc()->killed = 1;
      break;
```

-------------------------------------------------------------------------------------------------------------------------------------------------------------------

### TEST
##### In `Makefile`
* Add `nullpointer.c`

Our test file `nullpointer.c` include two function to test :

* Null pointer derefrence.

![Screenshot from 2021-01-25 12-50-39](https://user-images.githubusercontent.com/47724391/105696427-fb47d580-5f0b-11eb-91a2-d0bbef7fdaf2.png)

* Copying parent memory to child memory from second page.

![Screenshot from 2021-01-25 12-52-04](https://user-images.githubusercontent.com/47724391/105696557-29c5b080-5f0c-11eb-9691-03c726bd4ed8.png)

------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Read-only Code
In XV6 code is marked read-write but In most operating systems code is marked read-only instead of read-write, so no program can overwrite its code.

To convert XV6 to be read-only or return it to be read-write again we have to change the protection bits of parts of the page table to be read-only, thus preventing such over-writes, and also be able to change them back.

 ![Screenshot from 2021-01-25 18-23-25](https://user-images.githubusercontent.com/47724391/105733677-7922d580-5f3a-11eb-88ec-2d12695e3bfd.png)
 
In page table entry (PTE) the writable bit is responsible for changing code to be read-only or read-write
------------------------------------------------------------------------------------------------------------------------------------------------------------------
#### To reset writable bit (read-only) we wrote a system calls: `int mprotect(void *addr, int len)` 

which  changes the protection bits of the page range starting at `addr` and of `len` pages to be read only. Thus, the program could still read the pages in this range after `mprotect()` finishes, but a write to this region should cause a trap (and thus kill the process).

#### To set writable bit (read-write) we wrote a system calls: `int munprotect(void *addr, int len)`

Which does opposite operation of `int mprotect(void *addr, int len)` , sets the region back to both readable and writeable.
------------------------------------------------------------------------------------------------------------------------------------------------------------------
#### All steps required to implement `mprotect` same like `munprotect` our main focus will be at `proc.c`

To implement system call follow this link : [Here](https://gist.github.com/nirmohi0605/9f1a266bb630a148bb49)

##### In `proc.c`
* Added `int mprotect(void *addr, int len)`  as shown below :

```c
int
mprotect(void *addr, int len){      ///mprotect(start, 1) ; 
  struct proc *curproc = myproc();
  
  //Check if addr points to a region that is not currently a part of the address space
  if(len <= 0 || (int)addr+len*PGSIZE > curproc->sz){ 
    cprintf("\nwrong len\n");
    return -1;
  }

  //Check if addr is not page aligned
  if((int)(((int) addr) % PGSIZE )  != 0){
    cprintf("\nwrong addr %p\n", addr);
    return -1;
  }
 
  //loop for each page
  pte_t *pte;
  int i;
  for (i = (int) addr; i < ((int) addr + (len) *PGSIZE); i+= PGSIZE){ //  from start to end=(start+lenght)
    // Getting the address of the PTE in the current process's page table (pgdir)
    // that corresponds to virtual address (i)
    pte = walkpgdir(curproc->pgdir,(void*) i, 0);
    if(pte && ((*pte & PTE_U) != 0) && ((*pte & PTE_P) != 0) ){// check it's present and user 
      *pte = (*pte) & (~PTE_W) ; //Clearing the write bit 
      cprintf("\nPTE : 0x%p\n", pte);
    } else {
      return -1;
    }
  }
  //Reloading the Control register 3 with the address of page directory 
  //to flush TLB
  lcr3(V2P(curproc->pgdir));
return 0;
}
```

#### `munprotect` Same like `mprotect` but:
replace line
```c
 *pte = (*pte) & (~PTE_W) ; //Clearing the write bit 
```
with
```c
*pte = (*pte) | (PTE_W) ; //Setting the write bit 
 ```
------------------------------------------------------------------------------------------------------------------------------------------------------------------
### Important information :
##### In `mmu.h` 
Clarify how to get Page Directory ,Page Table Index and  Offset within Page from Virtual address 
```c
// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

// page directory index
#define PDX(va)         (((uint)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va)         (((uint)(va) >> PTXSHIFT) & 0x3FF)


// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size( 0 =4KB ^^^^ 1 = 4MB )
```
------------------------------------------------------------------------------------------------------------------------------------------------------------------
### TEST
##### In `Makefile`
* Add `nullpointer.c`

![Screenshot from 2021-01-25 20-00-40](https://user-images.githubusercontent.com/47724391/105746408-12a4b400-5f48-11eb-9c79-e2ac3c8a30e9.png)

------------------------------------------------------------------------------------------------------------------------------------------------------------------

# Kernel Threads
In this project, we'll be adding real kernel threads to xv6.

Specifically, in this project we are required to do the following:

1. We have to define a new system call to create a kernel thread, called `clone()`.
2. We have to define a new system call to to wait for a thread called `join()`.
3. We have to implement a simple ticket lock and three routines `initlock_t()`, `acquire_t()` and `release_t()` which initialize, acquire and release the `ticketlock`.
4. Then, we'll use `clone()`, `join()`, `initlock_t()`, `acquire_t()` and `release_t()` to build a little thread library, with a `thread_create()` `thread_join()` calls and `lock_init()`, `lock_acquire()` and `lock_release()` functions.

The `clone()` and `join()` system calls are necessary for introducing the notion of a thread in the xv6 kernel. The ticket lock mechanism is used to synchronize across multiple threads.

## 1. Implementing `clone()`

1. We will add a wrapper function in `sysproc.c` to make sure the user has provided the right number and type of arguments before forwarding the arguments to the actual system call.
    ```
    int
    sys_clone(void) 
    {
      void *fcn, *arg1, *arg2, *stack;
      
      //check if arguments is valid before calling clone syscall
      if (argptr(0, (void *)&fcn, sizeof(void *)) < 0)	return -1;
      if (argptr(1, (void *)&arg1, sizeof(void *)) < 0)	return -1;
      if (argptr(2, (void *)&arg2, sizeof(void *)) < 0)	return -1;
      if (argptr(3, (void *)&stack, sizeof(void *)) < 0)	return -1;

      return clone(fcn, arg1, arg2, stack);
    }
    ```

2. Now we will implement the `clone()` system call in `proc.c` by copying the body of the fork() system call into the body of our new clone() function and modify it.

    The full signature of the function is: `int clone(void(*fcn)(void *, void *), void *arg1, void *arg2, void *stack)`.

    The clone() system call creates a new thread-like process. It is very similar to fork() in that it creates a child process from the current process, but there are a few key differences:
    * The child process should share the address space of the original process (`fork()` creates a whole new copy of the address space).
    * `clone()` takes a pointer to a function (`void(*fcn)(void *, void *)`) and two arguments (`void *arg1, void *arg2`), and runs the function as soon as the child process starts (`fork()` starts the child at the same place in code as the parent).
    * The child process's stack (`void *stack`) should be in the address space of the original process (`fork()` creates a separate stack for the child process).

3. Now that we have the implementation of the clone system call, we need to glue it in to the rest of the operating system.


## 2. Implementing `join()`

1. We will add a wrapper function in `sysproc.c` to make sure the user has provided the right number and type of arguments before forwarding the arguments to the actual system call.
    ```
    int
    sys_join(void)
    {
      int stackArg;
      void **stack;
      
      stackArg = argint(0, &stackArg);
      stack = (void**) stackArg;

      return join(stack);
    }
    ```

2. Now we will implement the `join()` system call in `proc.c` by copying the body of the `wait()` system call into the body of our new `join()` function and modify it.

    The full signature of the function is: `int join(void** stack)`.
    
    The `join()` system call waits for a child thread that shares the address space with the calling process to exit. It returns the PID of waited-for child or -1 if none. The location of the child's user stack is copied into the argument stack (which can then be freed). It is very similar to `wait()` (which does the same thing, but for processes), but there are a fey key differences:
    * The thread to be joined shares the address space of its parent (a child thread will have the same `pgdir` attribute as its parent), so `join()` shouldn't touch the thread virtual memory because freeing it would break the parent process (`wait()` frees the virtual memory of the child process).
    * `join()` have to check if a process is a child thread of the current process by checking if its parent equal to the current process and have the same pgdir (`wait()` just needs to find a process whose parent is equal to the current process).
    
3. Now that we have the implementation of the `join()` system call, we need to glue it in to the rest of the operating system.


## 3. Implementing ticket lock
 With `clone` and `join` we have the ability to create and join threads, but we lack a way to protect data from being accessed by multiple threads simultaneously. To provide support    for synchronization we will add a spinning ticketlock to xv6.

 A `ticketlock` is one way to implement a mutex, but adds a little bit of complexity in order to improve fairness. Normal mutexes can have starvation issues, where one thread manages  to acquire the lock before other threads almost all the time, preventing other threads from getting access. A `ticketlock` has a `turn` and a `ticket`. When a thread wants to acquire the lock, it receives a `ticket` number. It then waits for the lock's `turn` to equal its ticket. Meanwhile, the lock increments the `turn` each time a thread releases the lock. This creates a simple FIFO queue system.

### To implement the ticket lock, we have to do the following:
1. Define the `ticketlock` structure, which user programs can use to declare and use ticketlocks.
   ```
    struct ticketlock
    {
        int next_ticket; // next ticket number to be given
        int current_turn; // current ticket number being served
        struct proc *proc; // process currently holding the lock
    };
   ```
2. Then we will implement three system calls `initlock_t()`, `acquire_t()` and `release_t()` which initialize, acquire and release the `ticketlock`.

   1. We will add a wrapper function for each system call in `sysproc.c`.
      ```
      int sys_initlock_t(void)
      {
        struct ticketlock *tl;
        if (argptr(0, (char**)&tl, sizeof(struct ticketlock*)) < 0) return -1;

        initlock_t(tl);
        return 0;
      }
      ```
      ___
      ```
      int sys_acquire_t(void)
      {
        struct ticketlock *tl;
        if (argptr(0, (char**)&tl, sizeof(struct ticketlock*)) < 0) return -1;

        acquire_t(tl);
        return 0;
      }
      ```
      ___
      ```
      int sys_release_t(void)
      {
        struct ticketlock *tl;
        if (argptr(0, (char**)&tl, sizeof(struct ticketlock*)) < 0) return -1;
         
        release_t(tl);
        return 0;
      }
      ```
      
      
   2. We will implement each system call in `proc.c`.
      ```
      void initlock_t(struct ticketlock *lk)
      {
          lk->next_ticket = 0;
          lk->current_turn = 0;
      }
      ```
      ___
      ```
      void acquire_t(struct ticketlock *lk)
      {
          cli(); //clear inturrupt flag (IF) Disable inturrupts
          int myTicket = fetch_and_add(&lk->next_ticket, 1);
    
          while (lk->current_turn != myTicket)
          ticket_sleep(lk); // to prevent busy waiting.
      }
      ```
      ___
      ```
      void release_t(struct ticketlock *lk)
      {
          fetch_and_add(&lk->current_turn, 1);
          wakeup(lk); // wakup on release and reacquire lock.
          sti(); //set inturrupt flag (IF) Enable inturrupts
      }
      ```
      ___
      
       ```
       // ticket_sleep is a helper function used in acquire_t() to prevent busy waiting.
      void ticket_sleep(void *chan)
      {
          struct proc *p = myproc();

          if (p == 0)
              panic("sleep");

          acquire(&ptable.lock);

          p->chan = chan;
          p->state = SLEEPING;
          sched();
          p->chan = 0;

          release(&ptable.lock);
        }
      ```
#### And Voila! now we have a fully working ticket lock that we can use to protect data from being accessed by multiple threads simultaneously.


## 4. Adding Thread Library
 Now we can say that we are ready to use `clone()`, `join()`, `initlock_t()`, `acquire_t()` and `release_t()` to build a little thread library, with a `thread_create()` `thread_join()` calls and `lock_init()`, `lock_acquire()` and `lock_release()` functions.
 
 ### To make the thread library, we have to do the following:
 1. Create 5 functions in `ulib.c` that call the five system calls we made previously.
    ```
    int thread_create(void (*start_routine)(void*, void*), void *arg1, void *arg2)
    {
        void *stack = sbrk(PGSIZE);
        return clone(start_routine, arg1, arg2, stack);
    }
    ```
    ___
    ```
    int thread_join()
    {
        void *stack;
        int result = join(&stack);	
        return result;
    }
    ```
    ___
    ```
    void lock_init(struct ticketlock *lock)
    {
        initlock_t(lock);
    }
    ```
    ___
    ```
    void lock_acquire(struct ticketlock *lock)
    {
        acquire_t(lock);
    }
    ```
    ___
    ```
    void lock_release(struct ticketlock *lock)
    {
        release_t(lock);
    }
    ```
    
 2. Add a function prototype in `user.h` for the five functions.
    ```
    int thread_create(void(*fcn)(void*, void*), void *arg1, void *arg2);
    int thread_join();
    void lock_init(struct ticketlock *lock);
    void lock_acquire(struct ticketlock *lock);
    void lock_release(struct ticketlock *lock);
    ```
 #### Now we can use this thread library to make multithreading programs in xv6.
 
 
## Testing
Now we can make multithreading programs that contain two or more parts that can run concurrently.

To test Kernel Threads we have made a program that uses the thread library (`threadtest.c`), the program contains the following two tests:
1. Single thread test.
2. Multi threads test.

To run `threadtest.c` just type `$ threadtest` in `qemu`.

![Kernel Threads Test](https://user-images.githubusercontent.com/47731377/105708876-ad3bcd80-5f1d-11eb-991d-b5f9108bd14b.png)

You can notice the fairness of `ticketlock` in the multi threads test (threads take turns in execution).
## References:

## Team Members:
> * Omar Gamal : [@O-Gamal]( https://github.com/O-Gamal )
> * Mariam Gad : [@Mariamgad]( https://github.com/Mariamgad)
> * Mostafa Amin : [@M0stafaAmin]( https://github.com/M0stafaAmin )
> * Mostafa Ayman : [@MostafaAE]( https://github.com/MostafaAE)
> * Mostafa Saad  : [@MostafaSaad7]( https://github.com/MostafaSaad7)
