#ifndef __USERPROG__
#define __USERPROG__

#define BUFFER_SIZE 32

#define N3  0
#define H2O 1
#define N   2
#define H2  3
#define O2  4
#define NO2 5

#define I_N3  0	// N3 injector
#define I_H2O 1 // H2O injector   

#define R1 0 // N3->N+N+N reaction
#define R2 1 // 2H2O->2H2+O2 reaction
#define R3 2 // N+O2->NO2 reaction

typedef struct t_atmosphere{
  int numprocs;
  // 6 possible molecules.
  // Initialized when instantiated.
  // Must follow order of:
  // N3
  // H2O
  // N
  // H2
  // O2
  // NO2
  sem_t molecules[6];		
} t_atmosphere;

typedef struct depot{
  int index;		// keep track of where to put/grab from
  char nums[10];        // the number char that each process currently has
} depot;

#define PRODUCER_FILE_TO_RUN "producer.dlx.obj"
#define CONSUMER_FILE_TO_RUN "consumer.dlx.obj"

#endif
