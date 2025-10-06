#ifndef __USERPROG__
#define __USERPROG__

#define BUFFER_SIZE 32

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
  char molecule_names[6][10];     
} t_atmosphere;

typedef struct depot{
  int index;		// keep track of where to put/grab from
  char nums[10];        // the number char that each process currently has
} depot;

#define PRODUCER_FILE_TO_RUN "producer.dlx.obj"
#define CONSUMER_FILE_TO_RUN "consumer.dlx.obj"

#endif
