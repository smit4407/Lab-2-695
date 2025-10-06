#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])  
{
  t_atmosphere *atmosphere;
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int atmosphere_location;
  int inj_count;

  if (argc != 5) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <atmosphere_location> <inj_count>\n");
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  atmosphere_location = dstrtol(argv[3], NULL, 10);
  inj_count = dstrtol(argv[4], NULL, 10);

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
  while(inj_count > 0){
    sem_signal(atmosphere->molecules[atmosphere_location]);
    if(atmosphere_location == 0){
      Printf("injector: An N3 molecule is created\n");
    }
    else if(atmosphere_location == 1){
      Printf("injector: An H2O molecule is created\n");
    } 
    else{
      Printf("ERROR: injector with PID %d does not have a valid location. Exiting\n", Getpid());
      Exit();
    }
    inj_count = inj_count - 1;
  } 

  // Signal the semaphore to tell the original process that we're done
  Printf("n3_injector: n3_injector is complete.\n", Getpid()); 
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) { 
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n"); 
    Exit(); 
  } 

}
