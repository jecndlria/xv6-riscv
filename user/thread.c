#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/thread.h"

#define PGSIZE 4096

int clone(void *stack);

void lock_init(struct lock_t *lock) {
  lock->locked = 0;
}

void lock_acquire(struct lock_t *lock) {
  while (__sync_lock_test_and_set(&lock->locked, 1) != 0); // spin-wait (do nothing)
}

void lock_release(struct lock_t *lock) {
  __sync_lock_release(&lock->locked);
}

int thread_create(void *(start_routine)(void*), void *arg) {
  printf("thread_create: Allocating stack\n");

  // Allocate extra memory to ensure we can align it
  void *stack = malloc(PGSIZE + PGSIZE - 1); // allocate extra PGSIZE-1 bytes
  if (stack == 0) {
    printf("thread_create: Failed to allocate stack\n");
    return -1;
  }

  // Align the stack pointer within the allocated block
  void *aligned_stack = (void*)(((uint64)stack + PGSIZE - 1) & ~(PGSIZE - 1));

  printf("thread_create: Creating thread\n");
  int pid = clone(aligned_stack);
  if (pid < 0) {
    printf("thread_create: Clone failed\n");
    free(stack);
    return -1;
  } else if (pid == 0) { // Child
    printf("thread_create: In child, calling start_routine\n");
    start_routine(arg);
    //free(stack); // Free the stack in the child after use
    printf("thread_create: Child exiting\n");
    exit(0);
  } else { // Parent
    printf("thread_create: Parent returning, pid = %d\n", pid);
    return pid;
  }

  return 0; // Should not reach here
}
