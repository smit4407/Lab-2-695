#ifndef __USERPROG__
#define __USERPROG__

#define BUFFER_SIZE 32

typedef struct t_atmosphere{
  int numprocs;
  sem_t n3;
  sem_t n;
  sem_t h2o;
  sem_t h2;
  sem_t o2;
  sem_t no2;
} t_atmosphere;

typedef struct depot{
  int index;		// keep track of where to put/grab from
  char nums[10];        // the number char that each process currently has
} depot;

#define PRODUCER_FILE_TO_RUN "producer.dlx.obj"
#define CONSUMER_FILE_TO_RUN "consumer.dlx.obj"
#define REACTION1_FILE_TO_RUN "reaction1.dlx.obj"
#define REACTION2_FILE_TO_RUN "reaction2.dlx.obj"
#define REACTION3_FILE_TO_RUN "reaction3.dlx.obj"
#define N3_INJECTOR_FILE_TO_RUN "n3_injector.dlx.obj"
#define H2O_INJECTOR_FILE_TO_RUN "h2o_injector.dlx.obj"

#endif
