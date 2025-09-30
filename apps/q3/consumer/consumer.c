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
  depot c_depot = {.index = 0, .nums = "XXXXXXXXXX"};   // space to store consumed items
  char product;
  char expected_item = '0';

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
  //Printf("spawn_me: This consumer is one of the %d processes you created.\n", buf->numprocs);
  Printf("spawn_me: PID %d is a consumer process you created of a total %d.\n", Getpid(), buf->numprocs);
  //Printf("spawn_me: My PID is %d\n", Getpid());
  
  while(c_depot.index < 10){
    ditoa(c_depot.index, &expected_item);
    // Printf("spawn_me: PID %d expected item: %d\n", Getpid(), dstrtol(&expected_item, NULL, 10));
    
    //sem_wait(buf->s_fullslots);
    if((res = lock_acquire(buf->lock)) == SYNC_FAIL){
      Printf("spawn_me: PID %d could NOT get lock! Res: %d\n", Getpid(), res);
    }
    // Buffer is empty
    //if((buf->w_idx) == (buf->r_idx)){
    //  lock_release(buf->lock);
    //  // Printf("spawn_me: PID %d found empty buffer.\n", Getpid());
    //}
    //else{
      // Head of buffer has what we are looking for
      // consume item and update index's
      if((buf->buffer[buf->r_idx]) == expected_item){
	sem_wait(buf->s_fullslots);
	product = buf->buffer[buf->r_idx];
	buf->r_idx = (buf->r_idx + 1) % BUFFER_SIZE;
	lock_release(buf->lock);
        sem_signal(buf->s_emptyslots);
	c_depot.nums[c_depot.index] = product;
	c_depot.index = c_depot.index + 1;
	//Printf("spawn_me: Consumer %d removed: %d\n", Getpid(), dstrtol(&product, 1, 10));
	// Using subtraction trick because dstrtol needs null terminated string
	Printf("spawn_me: Consumer %d removed: %d\n", Getpid(), (product - '0'));
      }
      // Head of buffer does not have what we are looking for
      // so release lock so another consumer can get item
      else{
        lock_release(buf->lock);
        Printf("spawn_me: PID %d did not find what it was looking for.\n", Getpid());
      }
    }
  //}

  //Printf("spawn_me: PID %d got the lock!\n", Getpid());
  //lock_release(buf->lock);

  // Signal the semaphore to tell the original process that we're done
  Printf("spawn_me: Consumer %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
