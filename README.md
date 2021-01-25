# Table of contents
* [Lottery Scheduler]()
* [Virtual Memory]()
* [Kernel Threads](#kernel-threads)
* [References](#references)
* [Team Members](#team-members)

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
