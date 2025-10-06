#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 0;               // Used to store number of processes to create
  t_atmosphere *atmosphere;   	 
  sem_t s;
  int i;
  int inj_count[2];
  int expected_reactions[3];
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char inj_count_str[2][10];
  char expected_reactions_str[2][10];
  int  inj_idx;
  int  reaction_idx;
  char inj_idx_str[10];
  char reaction_idx_str[10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf("<number of N3 molecules> <number of H20 molecules>\n");
    Exit();
  }

  
  // Hard coding this to 5 because there will always be two "injectors" 
  // processes and three "reactions" processes.
  numprocs = 5;
  // Printf("Creating %d processes\n", numprocs);
 
  inj_count[I_N3] = dstrtol(argv[1], NULL, 10);
  //Printf("makeprocs: Number of N3 molecules available %d\n", inj_count[I_N3]);
  inj_count[I_H2O] = dstrtol(argv[2], NULL, 10);
  //Printf("makeprocs: Number of H20 molecules available %d\n", inj_count[I_H2O]);

  // Reaction 1 is equal to the number of N3 molecules present.
  expected_reactions[R1] = inj_count[I_N3];
  // Reaction 2 is equal to half the number of H2O molecules present.
  expected_reactions[R2] = inj_count[I_H2O] / 2;
  // Reaction 3 is equal to the smaller of N versus O2 in atmosphere after other reactions occur.
  if((3*inj_count[I_N3])>expected_reactions[R2]){
    expected_reactions[R3] = expected_reactions[R2];
  }
  else{ 
    expected_reactions[R3] = (3*inj_count[I_N3]);
  }
  // Printf("makeprocs: Expected number of type 1 reactions are %d\n", expected_reactions[R1]);
  // Printf("makeprocs: Expected number of type 2 reactions are %d\n", expected_reactions[R2]);
  // Printf("makeprocs: Expected number of type 3 reactions are %d\n", expected_reactions[R3]);

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // Map shared memory page into this process's memory space
  if ((atmosphere = (t_atmosphere *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }

  // Create and init a semaphore and put it into shared memory
  // Init with 0 because we start with empty atmosphere
  for(i=0; i<6; i++){
    if ((s = sem_create(0)) == SYNC_FAIL){
      Printf("ERROR: makeprocs: Could not create semaphore! Exiting.\n");
      Exit();
    }
    atmosphere->molecules[i] = s;
  }

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(inj_count[I_N3], inj_count_str[I_N3]);
  ditoa(inj_count[I_H2O], inj_count_str[I_H2O]);
  ditoa(expected_reactions[R1], expected_reactions_str[R1]);
  ditoa(expected_reactions[R2], expected_reactions_str[R2]);
  ditoa(expected_reactions[R3], expected_reactions_str[R3]);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(inj_idx=0; inj_idx<2; inj_idx++){
    ditoa(inj_idx, inj_idx_str);
    process_create(PRODUCER_FILE_TO_RUN, h_mem_str, s_procs_completed_str, inj_idx_str, inj_count_str[inj_idx], NULL);
  //Printf("makeprocs: reaction1 created.\n");
  }

  for(reaction_idx=0; reaction_idx<3; reaction_idx++){
    ditoa(reaction_idx, reaction_idx_str);
    process_create(CONSUMER_FILE_TO_RUN, h_mem_str, s_procs_completed_str, reaction_idx_str, expected_reactions_str[reaction_idx], NULL);
  //Printf("makeprocs: reaction1 created.\n");
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
