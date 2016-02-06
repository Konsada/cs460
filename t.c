#include "util.h"
#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED 

typedef enum {FREE, READY, SLEEP, BLOCK, ZOMBIE} STATUS;

typedef struct proc{
  struct proc *next;
  int    *ksp;               // saved ksp when not running
  int    status;
  int    priority;
  int    pid;
  int    ppid;                // add pid for identify the proc
  struct proc   *parent;
  int    kstack[SSIZE];      // proc stack area
}PROC;

int  procSize = sizeof(PROC);

PROC proc[NPROC], *running, *freeList, *readyQueue;    // define NPROC procs
extern int color;
   
PROC *dequeue (PROC **queue);
int body();

int scheduler()
{
  if (running->status == READY)
    enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
}

// e.g. get_proc(&freeList);
PROC *get_proc(PROC **list){
  PROC *p;
  p = *list;
  while(p) {
    if(p->status == FREE) {
      return p;
    }
    p = p->next;
  }
  return 0;
}
// e.g. put_proc(&freeList, p);
int put_proc(PROC **list, PROC *p) {
  PROC *ptr;
  ptr = *list;
  while(ptr) {
  }
  ptr->next = p;
  return 0;
}

// linear queue with running proc in the highest priority 
int enqueue(PROC **queue, PROC *p) {
  PROC *prev, *cur;

  myprintf("enqueue()\n");
  prev = queue;
  myprintf("prev = %x\n", prev);
  if(!prev){
    queue = &p;
    return 1;
  }
  
  cur = prev->next;
  myprintf("cur = %x\n", cur);
  if(prev->priority > p->priority)
    prev->next = p;
  else{
    p->next = prev;
    queue = &p;
  }

  while(cur) {
    if(prev->priority < p->priority)
      break;
  }
  prev->next = p;
  p->next = cur;
  p->status = READY;
  myprintf("p->status(pid:%d) = READY\n", p->pid);
  return 1;
}
// remove a PROC with the highest priority (the first one in queue)
// returns its pointer
PROC *dequeue (PROC **queue) {
  PROC *p;
  p = queue;
  queue = &(p->next);

  return p;
}

int printQueue(PROC *queue) {
  myprintf("printQueue()\n");
  while(queue){
    myprintf("[%d, %d]->", queue->pid, queue->priority);
    queue = queue->next;
  }
  myprintf("NULL");
  return 0;
}
PROC *kfork() // create a child process, begin from body()
{
  int i;
  PROC *p = get_proc(&freeList);
  myprintf("kfork()\n");
  if(!p) {
    printf("no more PROC, kfork() failed\n");
    return 0;
  }
  p->status = READY;
  myprintf("p->status = READY\n");
  p->priority = 1;        //priority = 1 for all proc except P0
  myprintf("p->priority = 1\n");
  p->ppid = running->pid; //parent = running
  myprintf("p->ppid = %d\n", p->ppid);
  /* Initialize new proc's kstack[ ] */
  for (i = 1; i < 10; i++)          // saved CPU registers
    p->kstack[SSIZE - i] = 0; 
  p->kstack[SSIZE-1] = (int)body; // resume point = address of body()
  p->ksp = &p->kstack[SSIZE-9];   // proc saved sp
  enqueue(&readyQueue, p);        // enter p into readyQueue by priority
  myprintf("exit kfork()\n");
  return p;
}

int init()
{
   PROC *p;
   int i, j;
   myprintf("intit()\n");
   /* initialize all proc's */
   for (i=0; i<NPROC; i++){
       p = &proc[i];
       p->status = FREE;
       p->pid = i;                        // pid = 0,1,2,..NPROC-1
       printf("p->pid = %d\n", i);
       p->priority = 0;
       p->ppid = 0;
       p->parent = 0;
       if(i<NPROC-1)
       p->next = &proc[i+1];              // point to next proc
       else
	 p->next = 0;
       if (i){                            // not for P0
          p->kstack[SSIZE-1] = (int)body; // entry address of body()
          for (j=2; j<10; j++)            // kstack[ ] high end entries = 0
               p->kstack[SSIZE-j] = 0;
          p->ksp = &(p->kstack[SSIZE-9]);
       }
       else{
	 running = &proc[0];
	 p->status = READY;
	 p->parent = &proc[0];
       }
   }
   proc[NPROC-1].next = &proc[0];         // all procs form a circular link list

   running = &proc[0];                    // P0 is running 

   freeList = &proc[0];
   readyQueue = 0;
   myprintf("init complete\n");
 }

body()
{ 
  PROC *child = 0;
  char c = '\0';
  myprintf("proc %d resumes to body()\n", running->pid);
  while(c != 'q'){
    color = running->pid + 7;
    myprintf("proc %d running : enter a key[s|q|f] : ", running->pid);
    c = getc(); 
    myprintf("%c\n", c);
    switch(c){
    case 's': tswitch();
      break;
    case 'q': myprintf("quit\n");
      break;
    case 'f': 
      child = kfork();
      if(child)
	myprintf("Successfully forked %d\n", child->pid);
      else
	myprintf("Failed fork\n");
      break;
    }
  }
}

int main()
{
  printf("MTX starts in main()\n");
  init();
  kfork();
  myprintf("start loop\n");
  while(1){
    if(readyQueue){
      myprintf("readyQueue = %d\n", readyQueue);
      tswitch();
    }
  }
}
