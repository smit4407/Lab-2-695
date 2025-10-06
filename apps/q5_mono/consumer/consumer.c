#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  t_atmosphere *atmosphere;
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int expected_reactions;
  int atmosphere_location;

  if (argc != 5) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <atmosphere_location> <expected_reactions>\n");
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  atmosphere_location = dstrtol(argv[3], NULL, 10);
  expected_reactions = dstrtol(argv[4], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((atmosphere = (t_atmosphere *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
 
  // Now print a message to show that everything worked
  Printf("reaction: PID %d is a reaction process. Expected reactions %d\n", Getpid(), expected_reactions);
  
  while(expected_reactions > 0){
    Printf("reaction: An N molecule is created\n");
    expected_reactions = expected_reactions - 1;
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("reaction: PID %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
