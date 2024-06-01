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
  while (__sync_lock_test_and_set(&lock->locked, 1) != 0)
    ; // spin-wait (do nothing)
}

void lock_release(struct lock_t *lock) {
  __sync_lock_release(&lock->locked);
}

void thread_start(void *(start_routine)(void*), void *arg) {
  start_routine(arg);
  exit(0); // Thread exits when the start routine finishes
}

int thread_create(void *(start_routine)(void*), void *arg) {
  void *stack = malloc(PGSIZE);
  if (stack == 0) {
    return -1;
  }

  int pid = clone(stack);
  if (pid < 0) {
    free(stack);
    return -1;
  } else if (pid == 0) { // Child
    thread_start(start_routine, arg);
  } else { // Parent
    return pid;
  }

  return 0; // Should not reach here
}
