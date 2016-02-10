#include "util.h"
#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED 
#define NULL 0
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
  if(*list)
    return dequeue(&list);
  return 0;
}
// e.g. put_proc(&freeList, p);
int put_proc(PROC **list, PROC *p) {
  PROC *ptr;
  ptr = *list;

  p->status = FREE;
  if(*list){
    p->next = *list;
    *list = p;
  }
  else {
    *list = p;
    p->next = NULL;
  }
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
    //    prev = cur;
    //    cur = cur->next;
    myprintf("prev->pid:%d -> cur->pid:%d\n", prev, cur);
  }
  prev->next = p;
  p->next = cur;
  p->status = READY;
  printQueue(queue);
  myprintf("p->status(pid:%d) = READY\n", p->pid);
  return 1;
}
// remove a PROC with the highest priority (the first one in queue)
// returns its pointer
PROC *dequeue (PROC **queue) {
  PROC *p;
  p = *queue;
  if(*queue) {
  *queue = (p->next);
  }
  return p;
  
}

int exit(){
  if(running->pid == 0) {
    body();
  }
  else {
    running->status = ZOMBIE;
    tswitch();
  }
  return 0;
}
int printQueue(PROC *queue, char *queueName) {
  PROC *p = 0;
  myprintf("printQueue(%s)\n", queueName);
  p = queue;
  if(!p){
    myprintf("Queue is empty!\n");
    return 0;
  }
  while(p){
    myprintf("[%d, %d]->", p->pid, p->priority);
    p = p->next;
  }
  myprintf("NULL\n");
  return 1;
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
  p->priority = 1;        //priority = 1 for all proc except P0
  p->ppid = running->pid; //parent = running
  /* Initialize new proc's kstack[ ] */
  for (i = 1; i < 10; i++)          // saved CPU registers
    p->kstack[SSIZE - i] = 0; 
  p->kstack[SSIZE-1] = (int)body; // resume point = address of body()
  printQueue(freeList, "freeList");
  p->ksp = &(p->kstack[SSIZE-9]);   // proc saved sp
  enqueue(&readyQueue, p);        // enter p into readyQueue by priority
  myprintf("exit kfork()\n");
  return p;
}

int printProc(PROC *p) {
  if(p){
  myprintf("p->next:     %x\n", p->next);
  myprintf("p->ksp:      %x\n", p->ksp);
  myprintf("p->priority: %d\n", p->priority);
  myprintf("p->pid:      %d\n", p->pid);
  myprintf("p->ppid:     %d\n", p->ppid);
  myprintf("p->parent:   %x\n", p->parent);
  myprintf("p->kstack:   %x\n", p->kstack);
  getc();
  }
  else {
    myprintf("Process not found!\n");
    return 0;
  }
  return 1;
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
       p->priority = 0;
       p->ppid = 0;
       p->parent = 0;
       if(i < NPROC - 1)
	 p->next = &proc[i+1];
       else
	 p->next = 0;
       if (i){                            // not for P0
          p->kstack[SSIZE-1] = (int)body; // entry address of body()
          for (j=2; j<10; j++)            // kstack[ ] high end entries = 0
               p->kstack[SSIZE-j] = 0;
          p->ksp = &(p->kstack[SSIZE-9]);
       }
       printProc(p);
   }       
   running = &proc[0];
   p->status = READY;
   p->parent = &proc[0];
       
   
   proc[NPROC-1].next = NULL;         // all procs form a circular link list
   running = &proc[0];                    // P0 is running 
   printProc(running);
   freeList = &proc[1];
   printQueue(readyQueue, "readyQueue");
   readyQueue = &proc[0];
   myprintf("RUNNING PROC\n");
   printProc(running);
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
    case 'q': myprintf("exit()\n");
      running->status = ZOMBIE;
      tswitch();
      exit();
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
  myprintf("MTX starts in main()\n");
  init();
  printQueue(freeList, "freeList");
  printQueue(readyQueue, "readyQueue");
  kfork();
  tswitch();
  body();
}
