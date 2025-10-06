//
//	synch.c
//
//	Routines for synchronization
//
//

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"

static Sem sems[MAX_SEMS]; 	// All semaphores in the system
static Lock locks[MAX_LOCKS];   // All locks in the system
static Cond conds[MAX_CONDS];   // All conditional variables in the system

extern struct PCB *currentPCB; 
//----------------------------------------------------------------------
//	SynchModuleInit
//
//	Initializes the synchronization primitives: the semaphores
//----------------------------------------------------------------------
int SynchModuleInit() {
  int i; // Loop Index variable
  dbprintf ('p', "SynchModuleInit: Entering SynchModuleInit\n");
  for(i=0; i<MAX_SEMS; i++) {
    sems[i].inuse = 0;
  }
  for(i=0; i<MAX_LOCKS; i++) {
    // Your stuff for initializing locks goes here
    locks[i].inuse = 0;
  }
  for(i=0; i<MAX_CONDS; i++) {
    // Your stuff for initializing Condition variables goes here
    conds[i].inuse = 0;
  }
  dbprintf ('p', "SynchModuleInit: Leaving SynchModuleInit\n");
  return SYNC_SUCCESS;
}

//---------------------------------------------------------------------
//
//	SemInit
//
//	Initialize a semaphore to a particular value.  This just means
//	initting the process queue and setting the counter.
//
//----------------------------------------------------------------------
int SemInit (Sem *sem, int count) {
  if (!sem) return SYNC_FAIL;
  if (AQueueInit (&sem->waiting) != QUEUE_SUCCESS) {
    printf("FATAL ERROR: could not initialize semaphore waiting queue in SemInit!\n");
    exitsim();
  }
  sem->count = count;
  return SYNC_SUCCESS;
}

//----------------------------------------------------------------------
// 	SemCreate
//
//	Grabs a Semaphore, initializes it and returns a handle to this
//	semaphore. All subsequent accesses to this semaphore should be made
//	through this handle.  Returns SYNC_FAIL on failure.
//----------------------------------------------------------------------
sem_t SemCreate(int count) {
  sem_t sem;
  uint32 intrval;

  // grabbing a semaphore should be an atomic operation
  intrval = DisableIntrs();
  for(sem=0; sem<MAX_SEMS; sem++) {
    if(sems[sem].inuse==0) {
      sems[sem].inuse = 1;
      break;
    }
  }
  RestoreIntrs(intrval);
  if(sem==MAX_SEMS) return SYNC_FAIL;

  if (SemInit(&sems[sem], count) != SYNC_SUCCESS) return SYNC_FAIL;
  return sem;
}


//----------------------------------------------------------------------
//
//	SemWait
//
//	Wait on a semaphore.  As described in Section 6.4 of _OSC_,
//	we decrement the counter and suspend the process if the
//	semaphore's value is less than 0.  To ensure atomicity,
//	interrupts are disabled for the entire operation, but must be
//      turned on before going to sleep.
//
//----------------------------------------------------------------------
int SemWait (Sem *sem) {
  Link	*l;
  int		intrval;
    
  if (!sem) return SYNC_FAIL;

  intrval = DisableIntrs ();
  dbprintf ('I', "SemWait: Old interrupt value was 0x%x.\n", intrval);
  dbprintf ('s', "SemWait: Proc %d waiting on sem %d, count=%d.\n", GetCurrentPid(), (int)(sem-sems), sem->count);
  if (sem->count <= 0) {
    dbprintf('s', "SemWait: putting process %d to sleep\n", GetCurrentPid());
    if ((l = AQueueAllocLink ((void *)currentPCB)) == NULL) {
      printf("FATAL ERROR: could not allocate link for semaphore queue in SemWait!\n");
      exitsim();
    }
    if (AQueueInsertLast (&sem->waiting, l) != QUEUE_SUCCESS) {
      printf("FATAL ERROR: could not insert new link into semaphore waiting queue in SemWait!\n");
      exitsim();
    }
    ProcessSleep();
    // Don't decrement couter here because that's handled in SemSignal for us
  } else {
    sem->count--; // Decrement internal counter
    dbprintf('s', "SemWait: Proc %d granted permission to continue by sem %d\n", GetCurrentPid(), (int)(sem-sems));
  }
  RestoreIntrs (intrval);
  return SYNC_SUCCESS;
}

int SemHandleWait(sem_t sem) {
  if (sem < 0) return SYNC_FAIL;
  if (sem >= MAX_SEMS) return SYNC_FAIL;
  if (!sems[sem].inuse)    return SYNC_FAIL;
  return SemWait(&sems[sem]);
}

//----------------------------------------------------------------------
//
//	SemSignal
//
//	Signal on a semaphore.  Again, details are in Section 6.4 of
//	_OSC_.
//
//----------------------------------------------------------------------
int SemSignal (Sem *sem) {
  Link *l;
  int	intrs;
  PCB *pcb;

  if (!sem) return SYNC_FAIL;

  intrs = DisableIntrs ();
  dbprintf ('s', "SemSignal: Process %d Signalling on sem %d, count=%d.\n", GetCurrentPid(), (int)(sem-sems), sem->count);
  // Increment internal counter before checking value
  sem->count++;
  if (sem->count > 0) { // check if there is a process to wake up
    if (!AQueueEmpty(&sem->waiting)) { // there is a process to wake up
      l = AQueueFirst(&sem->waiting);
      pcb = (PCB *)AQueueObject(l);
      if (AQueueRemove(&l) != QUEUE_SUCCESS) { 
        printf("FATAL ERROR: could not remove link from semaphore queue in SemSignal!\n");
        exitsim();
      }
      dbprintf ('s', "SemSignal: Waking up PID %d.\n", (int)(GetPidFromAddress(pcb)));
      ProcessWakeup (pcb);
      // Decrement counter on behalf of woken up PCB
      sem->count--;
    }
  }
  RestoreIntrs (intrs);
  return SYNC_SUCCESS;
}

int SemHandleSignal(sem_t sem) {
  if (sem < 0) return SYNC_FAIL;
  if (sem >= MAX_SEMS) return SYNC_FAIL;
  if (!sems[sem].inuse)    return SYNC_FAIL;
  return SemSignal(&sems[sem]);
}

//-----------------------------------------------------------------------
//	LockCreate
//
//	LockCreate grabs a lock from the systeme-wide pool of locks and 
//	initializes it.
//	It also sets the inuse flag of the lock to indicate that the lock is
//	being used by a process. It returns a unique id for the lock. All the
//	references to the lock should be made through the returned id. The
//	process of grabbing the lock should be atomic.
//
//	If a new lock cannot be created, your implementation should return
//	INVALID_LOCK (see synch.h).
//-----------------------------------------------------------------------
lock_t LockCreate() {
  lock_t l;
  uint32 intrval;

  // grabbing a lock should be an atomic operation
  intrval = DisableIntrs();
  for(l=0; l<MAX_LOCKS; l++) {
    if(locks[l].inuse==0) {
      locks[l].inuse = 1;
      break;
    }
  }
  RestoreIntrs(intrval);
  if(l==MAX_LOCKS) return SYNC_FAIL;

  if (LockInit(&locks[l]) != SYNC_SUCCESS) return SYNC_FAIL;
  return l;
}

int LockInit(Lock *l) {
  if (!l) return SYNC_FAIL;
  if (AQueueInit (&l->waiting) != QUEUE_SUCCESS) {
    printf("FATAL ERROR: could not initialize lock waiting queue in LockInit!\n");
    exitsim();
  }
  l->pid = -1;
  return SYNC_SUCCESS;
}

//---------------------------------------------------------------------------
//	LockHandleAcquire
//
//	This routine acquires a lock given its handle. The handle must be a 
//	valid handle for this routine to succeed. In that case this routine 
//	returns SYNC_FAIL. Otherwise the routine returns SYNC_SUCCESS.
//
//---------------------------------------------------------------------------
int LockAcquire(Lock *k) {
  Link	*l;
  int		intrval;
    
  if (!k) return SYNC_FAIL;

  // Locks are atomic
  intrval = DisableIntrs ();
  dbprintf ('I', "LockAcquire: Old interrupt value was 0x%x.\n", intrval);

  // Check to see if the current process owns the lock
  if (k->pid == GetCurrentPid()) {
    dbprintf('s', "LockAcquire: Proc %d already owns lock %d\n", GetCurrentPid(), (int)(k-locks));
    RestoreIntrs(intrval);
    return SYNC_SUCCESS;
  }

  dbprintf ('s', "LockAcquire: Proc %d asking for lock %d.\n", GetCurrentPid(), (int)(k-locks));
  if (k->pid >= 0) { // Lock is already in use by another process
    dbprintf('s', "LockAcquire: putting process %d to sleep\n", GetCurrentPid());
    if ((l = AQueueAllocLink ((void *)currentPCB)) == NULL) {
      printf("FATAL ERROR: could not allocate link for lock queue in LockAcquire!\n");
      exitsim();
    }
    if (AQueueInsertLast (&k->waiting, l) != QUEUE_SUCCESS) {
      printf("FATAL ERROR: could not insert new link into lock waiting queue in LockAcquire!\n");
      exitsim();
    }
    ProcessSleep();
  } else {
    dbprintf('s', "LockAcquire: lock is available, assigning to proc %d\n", GetCurrentPid());
    k->pid = GetCurrentPid();
  }
  RestoreIntrs(intrval);
  return SYNC_SUCCESS;
}

int LockHandleAcquire(lock_t lock) {
  if (lock < 0) return SYNC_FAIL;
  if (lock >= MAX_LOCKS) return SYNC_FAIL;
  if (!locks[lock].inuse)    return SYNC_FAIL;
  return LockAcquire(&locks[lock]);
}

//---------------------------------------------------------------------------
//	LockHandleRelease
//
//	This procedure releases the unique lock described by the handle. It
//	first checks whether the lock is a valid lock. If not, it returns SYNC_FAIL.
//	If the lock is a valid lock, it should check whether the calling
//	process actually holds the lock. If not it returns SYNC_FAIL. Otherwise it
//	releases the lock, and returns SYNC_SUCCESS.
//---------------------------------------------------------------------------
int LockRelease(Lock *k) {
  Link *l;
  int	intrs;
  PCB *pcb;

  if (!k) return SYNC_FAIL;

  intrs = DisableIntrs ();
  dbprintf ('s', "LockRelease: Proc %d releasing lock %d.\n", GetCurrentPid(), (int)(k-locks));

  if (k->pid != GetCurrentPid()) {
    dbprintf('s', "LockRelease: Proc %d does not own lock %d.\n", GetCurrentPid(), (int)(k-locks));
    return SYNC_FAIL;
  }
  k->pid = -1;
  if (!AQueueEmpty(&k->waiting)) { // there is a process to wake up
    l = AQueueFirst(&k->waiting);
    pcb = (PCB *)AQueueObject(l);
    if (AQueueRemove(&l) != QUEUE_SUCCESS) { 
      printf("FATAL ERROR: could not remove link from lock queue in LockRelease!\n");
      exitsim();
    }
    dbprintf ('s', "LockRelease: Waking up PID %d, assigning lock.\n", (int)(GetPidFromAddress(pcb)));
    k->pid = GetPidFromAddress(pcb);
    ProcessWakeup (pcb);
  }
  RestoreIntrs (intrs);
  return SYNC_SUCCESS;
}

int LockHandleRelease(lock_t lock) {
  if (lock < 0) return SYNC_FAIL;
  if (lock >= MAX_LOCKS) return SYNC_FAIL;
  if (!locks[lock].inuse)    return SYNC_FAIL;
  return LockRelease(&locks[lock]);
}

//---------------------------------------------------------------------------
//	LockTransfer
//
//	This procedure transfers the lock to the process described by the PCB. It
//	first checks whether the lock is a valid lock. If not, it returns SYNC_FAIL.
//	If the lock is a valid lock, it should check whether the calling
//	process actually holds the lock. If not it returns SYNC_FAIL. Otherwise it
//	releases the lock, and returns SYNC_SUCCESS. If the target process is
//	waiting for the lock, it is woken up.
//---------------------------------------------------------------------------
int LockTransfer(Lock *k, PCB *pcb) {
  Link *l;
  int	intrs;

  if (!k) return SYNC_FAIL;

  intrs = DisableIntrs ();
  dbprintf ('s', "LockTransfer: Proc %d transferring lock %d.\n", GetCurrentPid(), (int)(k-locks));

  if (k->pid != GetCurrentPid()) {
    dbprintf('s', "LockTransfer: Proc %d does not own lock %d.\n", GetCurrentPid(), (int)(k-locks));
    return SYNC_FAIL;
  }

  k->pid = GetPidFromAddress(pcb);
  dbprintf ('s', "LockTransfer: Assigning lock to %d.\n", (int)(GetPidFromAddress(pcb)));
  l = AQueueFirst(&k->waiting);
  while (l) {
    pcb = (PCB *)AQueueObject(l);
    if (GetPidFromAddress(pcb) == k->pid) {
      if (AQueueRemove(&l) != QUEUE_SUCCESS) { 
        printf("FATAL ERROR: could not remove link from lock queue in LockTransfer!\n");
        exitsim();
      }
      dbprintf ('s', "LockTransfer: Waking up PID %d.\n", (int)(GetPidFromAddress(pcb)));
      ProcessWakeup (pcb);
      break;
    }
    l = AQueueNext(l);
  }
  RestoreIntrs (intrs);
  return SYNC_SUCCESS;
}

//--------------------------------------------------------------------------
//	CondCreate
//
//	This function grabs a condition variable from the system-wide pool of
//	condition variables and associates the specified lock with
//	it. It should also initialize all the fields that need to initialized.
//	The lock being associated should be a valid lock, which means that
//	it should have been obtained via previous call to LockCreate(). 
//	
//	If for some reason a condition variable cannot be created (no more
//	condition variables left, or the specified lock is not a valid lock),
//	this function should return INVALID_COND (see synch.h). Otherwise it
//	should return handle of the condition variable.
//--------------------------------------------------------------------------
cond_t CondCreate(lock_t lock) {
  // Your code goes here
  uint32 intrval;
  cond_t c;

  // Check if lock has been acquired
  if(locks[lock].inuse == 0){
    return INVALID_COND;
  }

  // Find a conditional variable that is not in use
  intrval = DisableIntrs();
  for(c = 0; c < MAX_CONDS; c++){
    if(conds[c].inuse == 0){
	conds[c].inuse = 1;
	conds[c].lock = lock;
        break;
    }
  }
  RestoreIntrs(intrval);
  if(c == MAX_CONDS) return INVALID_COND;
  if(AQueueInit(&conds[c].waiting) != QUEUE_SUCCESS){
    printf("FATAL ERROR: Could not initialize conditional variable waiting queue in CondCreate!\n");
    exitsim();
  }

  return c;
}

int CondWait(Cond *c){
  Link *l;
  uint32 intrval;
  PCB *pcb;

  intrval = DisableIntrs();
  dbprintf('s', "CondWait called!\n");
  // Confirm current process owns the lock
  if(locks[(c->lock)].pid != GetCurrentPid()){
    printf("CondWait: Proc can't wait, doesn't own lock!\n");
    dbprintf('s', "CondWait: Proc %d can't wait, doesn't own lock. Lock owner is %d\n", GetCurrentPid(), locks[(c->lock)].pid);
    RestoreIntrs(intrval);
    return SYNC_FAIL;
  }
  // If we make it here we own the lock and need to wait.
  // So we create add ourself to the waiting queue.
  if((l = AQueueAllocLink ((void *)currentPCB)) == NULL){
    printf("FATAL ERROR: Could not allocate link for wait queue in CondWait!\n");
    exitsim();
  }
  if(AQueueInsertLast (&c->waiting, l) != QUEUE_SUCCESS){
    printf("FATAL ERROR: Could not insert link for wait queue in CondWait!\n");
    exitsim();
  }

  // Release the lock and wakeup anyone sleeping   
  locks[c->lock].pid = -1;
  if (!AQueueEmpty(&locks[c->lock].waiting)) { // there is a process to wake up
    l = AQueueFirst(&locks[c->lock].waiting);
    pcb = (PCB *)AQueueObject(l);
    if (AQueueRemove(&l) != QUEUE_SUCCESS) {
      printf("FATAL ERROR: could not remove link from lock queue in LockRelease!\n");
      exitsim();
    }
    dbprintf ('s', "CondWait: Waking up PID %d, assigning lock.\n", (int)(GetPidFromAddress(pcb)));
    locks[c->lock].pid = GetPidFromAddress(pcb);
    ProcessWakeup (pcb);
  }

  ProcessSleep();
  RestoreIntrs(intrval);
  return SYNC_SUCCESS;
}

//---------------------------------------------------------------------------
//	CondHandleWait
//
//	This function makes the calling process block on the condition variable
//	till ConditionHandleSignal is
//	received. The process calling CondHandleWait must have acquired the
//	lock associated with the condition variable (the lock that was passed
//	to CondCreate. This implies the lock handle needs to be stored
//	somewhere. hint! hint!) for this function to
//	succeed. If the calling process has not acquired the lock, it does not
//	block on the condition variable, but a value of 1 is returned
//	indicating that the call was not successful. Return value of 0 implies
//	that the call was successful.
//
//	This function should be written in such a way that the calling process
//	should release the lock associated with this condition variable before
//	going to sleep, so that the process that intends to signal this
//	process could acquire the lock for that purpose. The process will be holding
//	the lock after woken up, since the process calling CondHandleSignal
//	transfers the lock to it.
//---------------------------------------------------------------------------
int CondHandleWait(cond_t c) {
  if (c < 0) return SYNC_FAIL;
  if (c >= MAX_SEMS) return SYNC_FAIL;
  if (!conds[c].inuse)    return SYNC_FAIL;
  return CondWait(&conds[c]);
}

int CondSignal(Cond *c){
  Link *l;
  uint32 intrval;
  PCB *pcb;

  intrval = DisableIntrs();
  if(locks[(c->lock)].pid != GetCurrentPid()){
    printf("CondWait: Proc can't wait, doesn't own lock!\n");
    dbprintf('s', "CondWait: Proc %d can't wait, doesn't own lock. Lock owner is %d\n", GetCurrentPid(), locks[(c->lock)].pid);
    RestoreIntrs(intrval);
    return SYNC_FAIL;
  }

  // Check if any process is waiting on this conditional variable
  if(!AQueueEmpty(&c->waiting)){ 
    // A process is waiting, transfer lock to it, put ourself in locks waiting queue, wake up waiting process, sleep ourself
    l = AQueueFirst(&c->waiting);
    pcb = (PCB *)AQueueObject(l);
    if(AQueueRemove(&l) != QUEUE_SUCCESS){
      printf("FATAL ERROR: Could not remove link from conditional queue in CondSignal!\n");
      exitsim();
    }
    dbprintf('s', "CondSignal: Waking up PID %d, trasfering lock from %d.\n", (int)(GetPidFromAddress(pcb)), GetCurrentPid());

    locks[(c->lock)].pid = GetPidFromAddress(pcb);

    if((l = AQueueAllocLink((void *)currentPCB)) == NULL){
      printf("FATAL ERROR: Could not allocate link for conditional lock queue in CondSignal!\n");
      exitsim();
    }
    if(AQueueInsertLast(&((locks[(c->lock)]).waiting), l) != QUEUE_SUCCESS){
      printf("FATAL ERROR: Could not insert link for conditional lock wait queue in CondSignal!\n");
      exitsim();
    }
    ProcessWakeup(pcb);
    ProcessSleep();
  }
  else{
    dbprintf('s', "CondSignal: No processes to wakeup. PID %d retains lock.\n", GetCurrentPid());
  }

  RestoreIntrs(intrval);
  return SYNC_SUCCESS;
}

//---------------------------------------------------------------------------
//	CondHandleSignal
//
//	This call wakes up exactly one process waiting on the condition
//	variable, if at least one is waiting. If there are no processes
//	waiting on the condition variable, it does nothing. In either case,
//	the calling process must have acquired the lock associated with
//	condition variable for this call to succeed, in which case it returns
//	0. If the calling process does not own the lock, it returns 1,
//	indicating that the call was not successful. If a process is woken up, the
//	calling process should transfer its lock to the process woken up, and put
//	itself into sleep. Note that the calling process must hold the lock when it
//	wakes up again, since it is still in the critical section.
//---------------------------------------------------------------------------
int CondHandleSignal(cond_t c) {
  if (c < 0) return SYNC_FAIL;
  if (c >= MAX_SEMS) return SYNC_FAIL;
  if (!conds[c].inuse)    return SYNC_FAIL;
  return CondSignal(&conds[c]);
}
