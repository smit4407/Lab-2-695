#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  t_atmosphere *atmosphere;
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int local_n3;

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_t    o_page_mapped_semaphore>\n");
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  local_n3 = dstrtol(argv[3], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((atmosphere = (t_atmosphere *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
 
  // Now print a message to show that everything worked
  //Printf("spawn_me: This producer is one of the %d processes you created.\n", buf->numprocs);
  Printf("n3_injector: PID %d is the N3 injector process.\n", Getpid());

  //Starting point
  // Printf("spawn_me: PID %d has index %d and data %d\n", Getpid(), p_depot.index, dstrtol(p_depot.nums, NULL, 10));

  // keep trying to "produce" until we have emptied our depot
  while(local_n3 > 0){
    sem_signal(atmosphere->n3);
    Printf("n3_injector: An N3 molecule is created\n");
    local_n3 = local_n3 - 1;
  } 

  // Signal the semaphore to tell the original process that we're done
  Printf("reaction_1: reaction_1 is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
