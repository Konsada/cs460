#include "type.h"
#include "util.h"

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
#include "hop.c"    	      // Problem 5.6

int color;

int init()
{
    PROC *p; int i;
    color = 0x0C;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler()
{
  if (running->status == RUNNING){
    running->status = READY;
    enqueue(&readyQueue, running);
  }
     running = dequeue(&readyQueue);
     running->status = RUNNING;
     color = running->pid + 0x0A;
     rflag = 0;
}

int int80h();
int set_vector(u16 vector, u16 handler)
{
     // put_word(word, segment, offset)
     put_word(handler, 0, vector<<2);
     put_word((0x1000/2),  0,(vector<<2) + 2);
}
            
int main()
{
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    set_vector(80, int80h);

    kfork("/bin/u1");     // P0 kfork() P1

    while(1){
      printf("P0 running\n");
      if(NPROC==2 && proc[1].status != READY)
	myprintf("no runable process, system halts\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      tswitch();         // P0 switch to run P1
   }
}

int printQueues() {
  PROC *p = 0;
  int i;
  for(i = 0; i < 40; i++) {
    myprintf("-");
  }
  myprintf("\n");
  myprintf("freelist    = ");
  p = freeList;
  //print freeList
  while(p && p->status == FREE) {
    myprintf(" %d ->", p->pid);
    p = p->next;
    //    getc();
  }
  myprintf(" NULL\n");

  p = readyQueue;
  myprintf("readyQueue  = ");
  //print readyQueue
  while(p && p->status == READY) {
    //    if(p != running)
      myprintf(" %d [%d ] ->",p->pid, p->priority);
    p = p->next;
    //    getc();
  }
  myprintf(" NULL\n");

  p = sleepList;
  myprintf("sleepList   =");
  //print sleepList
  while(p && p->status == SLEEP) {
    myprintf(" %d [ e=%d ] ->",p->pid, p->event);
    p = p->next;
    //    getc();
  }
  myprintf(" NULL\n");
  p = zombieList;
  myprintf("zombieList =");
  while(p && p->status == ZOMBIE) {
    myprintf(" %d [ e=%d ] ->", p->pid, p->exitCode);
    p = p->next;
    //    getc();
  }
  myprintf(" NULL\n");
  for(i = 0; i < 40; i++) {
    myprintf("-");
  }
  myprintf("\n");
} 

int printQueue(PROC *queue, char *queueName) {
  PROC *p = queue;

  if(!p){
    myprintf("%s is empty!\n", queueName);
    return 0;
  }

  myprintf("%s = ", queueName);

  while(p){
    myprintf("%d[%p]->", p->pid, p->status);
    //    myprintf("%d[p%d,e%d,x%d]->",p->pid, p->priority, p->event, p->exitCode);
    p = p->next;
  }
  myprintf("NULL\n");
  return 0;
}

