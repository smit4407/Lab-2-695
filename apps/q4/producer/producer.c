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
 
  // Now print a message to show that everything worked
  //Printf("spawn_me: This producer is one of the %d processes you created.\n", buf->numprocs);
  Printf("spawn_me: PID %d is a producer process you created of a total %d.\n", Getpid(), buf->numprocs);
  //Printf("spawn_me: My PID is %d\n", Getpid());

  //Starting point
  // Printf("spawn_me: PID %d has index %d and data %d\n", Getpid(), p_depot.index, dstrtol(p_depot.nums, NULL, 10));

  // keep trying to "produce" until we have emptied our depot
  while(p_depot.index < 10){
    // get item for consumer
    product = p_depot.nums[p_depot.index]; 

    // now we need to interact with shared memory, acquire lock
    if((res = lock_acquire(buf->lock)) == SYNC_FAIL){
      Printf("spawn_me: PID %d could NOT get lock! Res: %d", Getpid(), res);
    }
    if(((buf->w_idx + 1) % BUFFER_SIZE) == buf->r_idx){
    	// circular buffer is full do not write to it
	lock_release(buf->lock);
    }
    else{
    // lock acquired insert data and increment write index
    buf->buffer[buf->w_idx] = product;
    buf->w_idx = (buf->w_idx + 1) % BUFFER_SIZE;
    
    // done interacting with shared memory, release lock
    lock_release(buf->lock);
    Printf("spawn_me: Producer %d inserted: %d\n", Getpid(), dstrtol(&product, NULL, 10));
    //Printf("spawn_me: Producer %d index is: %d\n", Getpid(), p_depot.index);
    p_depot.index = p_depot.index + 1;
    }
  } 

  //Printf("spawn_me: PID %d got the lock!\n", Getpid());
  //lock_release(buf->lock);

  // Signal the semaphore to tell the original process that we're done
  Printf("spawn_me: Producer %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
