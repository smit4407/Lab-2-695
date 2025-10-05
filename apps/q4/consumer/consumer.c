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
  //Printf("consumer: This consumer is one of the %d processes you created.\n", buf->numprocs);
  //Printf("consumer: PID %d is a consumer process you created of a total %d.\n", Getpid(), buf->numprocs);
  //Printf("consumer: My PID is %d\n", Getpid());
  
  while(c_depot.index < 10){
    ditoa(c_depot.index, &expected_item);
    // Printf("consumer: PID %d expected item: %d\n", Getpid(), dstrtol(&expected_item, NULL, 10));
    
    // Busy checking for produced data
    // first thing we must do is get lock
    if((res = lock_acquire(buf->lock)) == SYNC_FAIL){
      Printf("consumer: PID %d could NOT get lock! Res: %d\n", Getpid(), res);
    }
    // Check empty buffer and head character is what we need
    if((buf->w_idx) == (buf->r_idx) || (buf->buffer[buf->r_idx] != expected_item)){
      if(buf->w_idx == buf->r_idx){ Printf("consumer: PID %d found empty buffer.\n", Getpid()); }
      if(buf->buffer[buf->r_idx] != expected_item){ Printf("consumer: PID %d expected %d not found. Head has %d\n", Getpid(), expected_item - '0', (buf->buffer[buf->r_idx]) - '0'); }
      cond_wait(buf->not_empty);
    }

    // Printf("consumer: PID %d has passed it's while loop.\n", Getpid());
    // Head of buffer has what we are looking for
    // consume item and update index's
    product = buf->buffer[buf->r_idx];
    buf->r_idx = (buf->r_idx + 1) % BUFFER_SIZE;
    Printf("Consumer %d removed: %d\n", Getpid(), (product - '0'));
    cond_signal(buf->not_full);
    lock_release(buf->lock);
    c_depot.nums[c_depot.index] = product;
    c_depot.index = c_depot.index + 1;
    Printf("consumer: Consumer %d end of loop has index %d and data %d.\n", Getpid(), c_depot.index, dstrtol(c_depot.nums, NULL, 10)); 
    //Printf("consumer: Consumer %d removed: %d\n", Getpid(), dstrtol(&product, 1, 10));
    // Using subtraction trick because dstrtol needs null terminated string
    // Head of buffer does not have what we are looking for
    // so release lock so another consumer can get item
    // Printf("consumer: PID %d did not find what it was looking for.\n", Getpid());
  }

  //Printf("consumer: PID %d got the lock!\n", Getpid());
  //lock_release(buf->lock);

  // Signal the semaphore to tell the original process that we're done
  // Printf("consumer: Consumer %d is complete.\n", Getpid()); 
  Printf("consumer: Consumer %d is complete with data %d.\n", Getpid(), dstrtol(c_depot.nums, NULL, 10)); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
