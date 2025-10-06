#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  t_atmosphere *atmosphere;
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int expected_reactions_1;

  if (argc != 4) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <expected_reactions_1>\n");
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  expected_reactions_1 = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((atmosphere = (t_atmosphere *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
 
  // Now print a message to show that everything worked
  Printf("reaction_1: PID %d is a reaction 1 process.\n", Getpid());
  
  while(expected_reactions_1 > 0){
    sem_wait(atmosphere->n3);
    // If we pass here we can perform the reaction (i.e. signal n three times)
    sem_signal(atmosphere->n);
    Printf("reaction_1: An N molecule is created\n");
    sem_signal(atmosphere->n);
    Printf("reaction_1: An N molecule is created\n");
    sem_signal(atmosphere->n);
    Printf("reaction_1: An N molecule is created\n");
    expected_reactions_1 = expected_reactions_1 - 1;
  }

  // Signal the semaphore to tell the original process that we're done
  Printf("reaction_1: reaction_1 is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
