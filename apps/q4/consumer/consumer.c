#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  shared_buffer *buf;      // Used to access buffer in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int i;
  char product;
  lock_t lock; 
  cond_t cond_not_full;      // CV for when buffer is not full
  cond_t cond_not_empty;     // CV for when buffer is not empty

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_t    o_page_mapped_semaphore>\n");
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((buf = (shared_buffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  lock = buf->lock;
  cond_not_full = buf->cond_not_full;
  cond_not_empty = buf->cond_not_empty;
  
  for (i = 0; i < 10; i++) {

    //get lock for consumer
    if (lock_acquire(lock) != SYNC_SUCCESS) {
      Printf("FATAL ERROR: Consumer %d could not acquire lock!\n", Getpid());
      Exit();
    }

    //wait while buffer empty
    while (buf->count == 0) {
      cond_wait(cond_not_empty);
    }

    //remove item from buffer
    product = buf->buffer[buf->head];
    Printf("Consumer %d removed: %c\n", Getpid(), product);

    //update buffer
    buf->head = (buf->head + 1) % BUFFER_SIZE;
    buf->count--;

    //wake up producer
    cond_signal(cond_not_full);

    if (lock_release(lock) != SYNC_SUCCESS) {
      Printf("FATAL ERROR: Consumer %d could not release lock!\n", Getpid());
      Exit();
    }
    
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("spawn_me: Consumer %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
