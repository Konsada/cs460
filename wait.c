//#include "type.h"

void ksleep(int event){
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //  reschedule();
  tswitch();
}

int kwait(int *status){
  int i = 0, hasChild = 0 ;
  PROC *p;

  while(1){
    for(i = 0; i < NPROC; i++){
      p = &proc[i];
      if(p->ppid == running->pid && p->status != FREE){
	hasChild = 1;
	if(p->status == ZOMBIE){
	  *status = p->exitCode;
	  p->status = FREE;
	  put_proc(&freeList, p);
	  nproc--;
	  return(p->pid);
	}
      }
    }
    if(!hasChild)
      return (-1);
    ksleep(running);
  }
}

void ready(PROC *p) {
  p->event = 0;
  p->status = READY;
  enqueue(&readyQueue, p);
  myprintf("wakeup proc %d\n", p->pid);
}

void kwakeup(int event){
  int i = 0;
  PROC *p, *tempQueue = 0;

  while(p = dequeue(&sleepList)){
    if(p->event == event){
      p->status = READY;
      enqueue(&readyQueue, p);
      myprintf("wakeup %d\n", p->pid);
      continue;
    }
    enqueue(&tempQueue, p);
  }
  sleepList = tempQueue;
}

int kexit(int exitValue){
  int i, wakeupP1 = 0;
  PROC *p;

  for(i = 1; i < NPROC; i++){
    p = &proc[i];
    if(p->status != FREE && p->ppid == running->pid){
      p->ppid = 1;
      p->parent = &proc[1];
    }
  }

  running->exitCode = exitValue;
  running->status = ZOMBIE;

  kwakeup(running->parent);
  kwakeup(&proc[1]);
  tswitch();
}
