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

  if((res = lock_acquire(buf->lock)) == SYNC_FAIL){
    Printf("spawn_me: PID %d could NOT get lock! Res: %d", Getpid(), res);
  }

  Printf("spawn_me: PID %d got the lock!\n", Getpid());
  lock_release(buf->lock);

  // Signal the semaphore to tell the original process that we're done
  Printf("spawn_me: PID %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
