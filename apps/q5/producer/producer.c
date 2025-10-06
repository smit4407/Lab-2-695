#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  t_atmosphere *atmosphere;
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int inj_idx;
  int inj_count;

  if (argc != 5) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <inj_idx> <inj_count>\n");
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  inj_idx = dstrtol(argv[3], NULL, 10);
  inj_count = dstrtol(argv[4], NULL, 10);

  // Map shared memory page into this process's memory space
  if ((atmosphere = (t_atmosphere *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
 
  // Now print a message to show that everything worked
  // Printf("injector: PID %d is an injector process. Injection count %d\n", Getpid(), inj_count);

  // keep trying to "produce" until we have emptied our depot
  while(inj_count > 0){
    sem_signal(atmosphere->molecules[inj_idx]);
    if(inj_idx == I_N3){
      Printf("An N3 molecule is created\n");
    }
    else if(inj_idx == I_H2O){
      Printf("An H2O molecule is created\n");
    } 
    else{
      Printf("ERROR: injector with PID %d does not have a valid index. Exiting\n", Getpid());
      Exit();
    }
    inj_count = inj_count - 1;
  } 

  // Signal the semaphore to tell the original process that we're done
  // Printf("injector: PID %d is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 
}
