#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  shared_buffer *buf;      // Used to access buffer in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int res;
  depot p_depot = {.index = 0, .nums = "0123456789"};
  char product;

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
 
  // Printf("producer: PID %d is a producer process you created of a total %d.\n", Getpid(), buf->numprocs);

  // keep trying to "produce" until we have emptied our depot
  while(p_depot.index < 10){
    // get item for consumer
    product = p_depot.nums[p_depot.index]; 

    sem_wait(buf->s_emptyslots);
    // now we need to interact with shared memory, acquire lock
    if((res = lock_acquire(buf->lock)) == SYNC_FAIL){
      Printf("producer: PID %d could NOT get lock! Res: %d", Getpid(), res);
    }
    buf->buffer[buf->w_idx] = product;
    buf->w_idx = (buf->w_idx + 1) % BUFFER_SIZE;
    
    // done interacting with shared memory, release lock
    lock_release(buf->lock);
    sem_signal(buf->s_fullslots);
    Printf("Producer %d inserted: %d\n", Getpid(), dstrtol(&product, NULL, 10));
    p_depot.index = p_depot.index + 1;
  } 

  // Signal the semaphore to tell the original process that we're done
  // Printf("Producer %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 
}
