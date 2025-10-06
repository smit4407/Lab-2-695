#ifndef __USERPROG__
#define __USERPROG__

#define BUFFER_SIZE 32

typedef struct shared_buffer{
  int numprocs;
  int w_idx;
  int r_idx;
  char buffer[BUFFER_SIZE]; 
  lock_t lock;
  sem_t s_fullslots;
  sem_t s_emptyslots;
} shared_buffer;

typedef struct depot{
  int index;		// keep track of where to put/grab from
  char nums[10];        // the number char that each process currently has
} depot;

#define PRODUCER_FILE_TO_RUN "producer.dlx.obj"
#define CONSUMER_FILE_TO_RUN "consumer.dlx.obj"

#endif
