#ifndef __USERPROG__
#define __USERPROG__

#define BUFFER_SIZE 10

typedef struct shared_buffer{
  int head;
  int count;
  int tail;
  char buffer[BUFFER_SIZE]; 	// fixed size because malloc not available!
  lock_t lock;
  cond_t cond_not_empty;
  cond_t cond_not_full;
} shared_buffer;

typedef struct depot{
  int index;		// keep track of where to put/grab from
  char nums[10];        // the number char that each process currently has
} depot;

#define PRODUCER_FILE_TO_RUN "producer.dlx.obj"
#define CONSUMER_FILE_TO_RUN "consumer.dlx.obj"

#endif
