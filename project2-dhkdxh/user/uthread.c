//
// User-space helpers for kernel threads.
//
// thread_create() and thread_join() are thin wrappers over the clone()
// and join() system calls.  They mirror the spirit of pthread_create()
// and pthread_join():

#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"
#include "user/uthread.h"

int
thread_create(void (*fn)(void *), void *arg, int n_pages)
{
  if(n_pages <= 0) return -1; //잘못된 입력 거르기

  void* stack = malloc(n_pages*PGSIZE);
  if(stack == 0) return -1;

  int pid = clone(fn, arg, stack, n_pages, CLONE_VM); //thread clone
  if(pid < 0){ //clone 안되면 할당 해제
    free(stack); return -1;
  }
  return pid;
}

int
thread_join(void)
{
  void *stack;
  int pid = join(&stack);

  if(pid < 0) return -1;
  free(stack);
  return pid;
}
