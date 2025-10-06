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
  //int n3_count;			  // Number of n3 molecules user inputs. Passed to injector.
  //int h2o_count;		  // Number of h2o molecules user inputs. Passed to injector.
  int inj_count[2];
  //int expected_reactions_1;        // Expected number of type 1 reactions.
  //int expected_reactions_2;       // Expected number of type 2 reactions.
  //int expected_reactions_3;      // Expected number of type 3 reactions.
  int expected_reactions[3];
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  //char n3_count_str[10];
  //char h2o_count_str[10];
  //char expected_reactions_1_str[10];
  //char expected_reactions_2_str[10];
  //char expected_reactions_3_str[10];
  char inj_count_str[2][10];
  char expected_reactions_str[2][10];

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf("<number of N3 molecules> <number of H20 molecules>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  // Multiply by 2 because the command line arugment is number of producer consumer pairs
  // numprocs = dstrtol(argv[1], NULL, 10) * 2; // the "10" means base 10
  // Printf("Creating %d processes\n", numprocs);
  
  // Hard coding this to 5 because there will always be two "injectors" 
  // processes and three "reactions" processes.
  numprocs = 5;
  //n3_count = dstrtol(argv[1], NULL, 10);
  inj_count[0] = dstrtol(argv[1], NULL, 10);
  //Printf("makeprocs: Number of N3 molecules available %d\n", n3_count);
  Printf("makeprocs: Number of N3 molecules available %d\n", inj_count[0]);
  //h2o_count = dstrtol(argv[1], NULL, 10);
  inj_count[1] = dstrtol(argv[1], NULL, 10);
  //Printf("makeprocs: Number of H20 molecules available %d\n", h2o_count);
  Printf("makeprocs: Number of H20 molecules available %d\n", inj_count[1]);

  // The number of N3->N+N+N reactions is simply the number of N3 molecules present.
  //expected_reactions_1 = n3_count;
  expected_reactions[0] = inj_count[0];
  // The number of 2H2O->2H2+O2 reactions is the result of (h2o_count/2).
  //expected_reactions_2 = h2o_count / 2;
  expected_reactions[1] = inj_count[1] / 2;
  if((3*inj_count[0])>expected_reactions[1]){
    //expected_reactions_3 = expected_reactions_2;
    expected_reactions[2] = expected_reactions[1];
  }
  else{ 
    //expected_reactions_3 = (3*n3_count);
    expected_reactions[2] = (3*inj_count[0]);
  }
  Printf("makeprocs: Expected number of type 1 reactions are %d\n", expected_reactions[0]);
  Printf("makeprocs: Expected number of type 2 reactions are %d\n", expected_reactions[1]);
  Printf("makeprocs: Expected number of type 3 reactions are %d\n", expected_reactions[2]);

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
  
  //atmosphere->molecule_names = {{"N3\0"}, {"H2O\0"}, {"N\0"}, {"H2\0"}, {"O2\0"}, {"NO2\0"}};
  //atmosphere->molecule_names[0] = "N3\0";
  //atmosphere->molecule_names[1] = "H2O\0";
  //atmosphere->molecule_names[2] = "N\0";
  //atmosphere->molecule_names[3] = "H2\0";
  //atmosphere->molecule_names[4] = "O2\0";
  //atmosphere->molecule_names[5] = "NO2\0";
  dstrcpy(atmosphere->molecule_names[0], "N3");
  dstrcpy(atmosphere->molecule_names[1], "H2O");
  dstrcpy(atmosphere->molecule_names[2], "N");
  dstrcpy(atmosphere->molecule_names[3], "H2");
  dstrcpy(atmosphere->molecule_names[4], "O2");
  dstrcpy(atmosphere->molecule_names[5], "NO2");

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
  //ditoa(n3_count, n3_count_str);
  //ditoa(h2o_count, h2o_count_str);
  //ditoa(expected_reactions_1, expected_reactions_1_str);
  //ditoa(expected_reactions_2, expected_reactions_2_str);
  //ditoa(expected_reactions_3, expected_reactions_3_str);
  ditoa(inj_count[0], inj_count_str[0]);
  ditoa(inj_count[1], inj_count_str[1]);
  ditoa(expected_reactions[0], expected_reactions_str[0]);
  ditoa(expected_reactions[1], expected_reactions_str[1]);
  ditoa(expected_reactions[2], expected_reactions_str[2]);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<2; i++){
    char i_str[10];
    ditoa(i, i_str);
    process_create(PRODUCER_FILE_TO_RUN, h_mem_str, s_procs_completed_str, i_str, inj_count_str[i], NULL);
  //Printf("makeprocs: reaction1 created.\n");
  }

  for(i=2; i<6; i++){
    char i_str[10];
    ditoa(i, i_str);
    if(i != 3){
      process_create(CONSUMER_FILE_TO_RUN, h_mem_str, s_procs_completed_str, i_str, expected_reactions_str[i-2], NULL);
    }
  //Printf("makeprocs: reaction1 created.\n");
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
