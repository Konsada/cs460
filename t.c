#include "util.h"
#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED 
#define NULL 0
typedef enum {FREE, READY, SLEEP, BLOCK, ZOMBIE} STATUS;

typedef struct proc{
  struct proc *next;
  int    ksp;               // saved ksp when not running
  int    status;
  int    priority;
  int    pid;
  int    ppid;                // add pid for identify the proc
  struct proc   *parent;
  int    kstack[SSIZE];      // proc stack area
  int event;
  int exitCode;
}PROC;

int  procSize = sizeof(PROC);
int nproc = 0;
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;    // define NPROC procs
extern int color;
char* string[30];

int enqueue(); 
PROC *dequeue (PROC **queue);
int body();
void ksleep(int event);
void kwakeup(int event);
int kexit(int exitValue);
int kwait(int *status); 
void do_tswitch();
int do_kfork();
PROC *kfork();
void do_exit();
void do_sleep();
void do_wakeup();
void do_wait();

void do_tswitch(){
  tswitch();
}

int do_kfork(){
  PROC *child, *p;
  child = kfork();
  if(child){
    return p->pid;
  }
  else
    myprintf("Failed fork\n");
  return -1;
}

void do_exit(){
  int exit = 0;
  myprintf("Please enter an exit value: \n");
  if(exit = getint()){
    running->status = ZOMBIE;
    tswitch();
    kexit();
  }
  else
    myprintf("Exit value not recognized!\n");
  return;
}

void do_sleep(){
  int event = 0;
  myprintf("Please enter an event value: \n");

  event = getint(gets());
  if(event)
    ksleep(event);
  else
    myprintf("Event not recognized!\n");
  return;
}

void do_wakeup(){
  int event = 0;
  char *input;

  myprintf("Please enter an event value: \n");

  input = gets();
  event = getint(input);

  if(event){
    kwakeup(event);
  }
  else{
    myprintf("event: %d\n", event);
    myprintf("Event not recognized!\n");
  }
  return;

}
void do_wait(){
  int pid, status;
  pid = kwait(&status);
  myprintf("waiting [pid: %d | status %d]\n", pid, status);
}

void ksleep(int event){
  running->event = event;
  running->status = SLEEP;
  tswitch();
}
void kwakeup(int event){
  int i = 0;


  for(i = 1; i < NPROC; i++) {

    if(proc[i].status == SLEEP && proc[i].event == event) {
      proc[i].event = 0;
      proc[i].status = READY;
      enqueue(&readyQueue, proc[i]);
    }
  }
}

int kexit(int exitValue){
  int i, wakeupP1 = 0;
  PROC *p;
  if (running->pid == 1){
    myprintf("other procs still exist, P1 can't die yet!\n");
    return -1;
  }
  for(i = 1; i < NPROC; i++) {
    p = &proc[i];
    if (p->status != FREE && p->ppid == running->pid) {
      p->ppid = 1;
      p->parent = &proc[1];
      wakeupP1++;
    }
  }
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  /* wakeup parent and also P1 if necessary */
  kwakeup(running->parent);
  if(wakeupP1)
    kwakeup(&proc[1]);
  tswitch();
}

int kwait(int *status){
  int i = 0, hasChild = 0 ;
  PROC *p;
  while(1){
    for(i = 1; i < NPROC; i++) {
      p = &proc[i];
      if(p->status != FREE && p->ppid == running->pid) {
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
    if(!hasChild) return -1;
    ksleep(running);
  }
}


int scheduler()
{

  //  printf("running status %d", running->status);
  if (running->status == READY){
    enqueue(&readyQueue, running);
    //   myprintf("running->pid: %d enqueued\n", running->pid);
  }
  else{
    //    running = dequeue(&readyQueue);
    put_proc(&readyQueue, running);
  }
  //  printQueue(readyQueue, "readyQueue");
  running = dequeue(&readyQueue);
  //  printQueue(readyQueue, "readyQueue");
}

// e.g. get_proc(&freeList);
PROC *get_proc(PROC **list){
  if(*list)
    return dequeue(&(*list));
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
  PROC *temp;

  temp = *queue;

  //empty queue
  if(!temp) {
    *queue = p;
    p->next = NULL;
    return 0;
  }
  //queue has only one element
  if(!temp->next) {
    if(temp->priority < p->priority) {
      p->next = temp;
      *queue = p;
      return 0;
    }
  }
  while(temp->next && temp->next->priority >= p->priority) {
    temp = temp->next; 
  }
  p->next = temp->next;
  temp->next = p;
  return 0;
}

// remove a PROC with the highest priority (the first one in queue)
// returns its pointer
PROC *dequeue (PROC **queue) {
  PROC *p = *queue;
  //myprintf("dequeue()\n");
  //myprintf("queue->pid = %d\n", (*queue)->pid);
  //printProc(freeList);
  if(*queue) {
    *queue = (*queue)->next;
  }
  p->next = NULL;
  //  printProc(p);
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
  while(p) {
    myprintf(" %d ->", p->pid);
    p = p->next;
  }
  myprintf(" NULL\n");

  p = readyQueue;
  myprintf("readyQueue  = ");
  //print readyQueue
  while(p) {
    myprintf(" %d [%d ] ->",p->pid, p->priority);
    p = p->next;
  }
  myprintf(" NULL\n");

  p = sleepList;
  myprintf("sleepList   =");
  //print sleepList
  while(p) {
    myprintf(" %d [ e=%d ] ->",p->pid, p->event);
    p = p->next;
  }
  myprintf(" NULL\n");
  for(i = 0; i < 40; i++) {
    myprintf("-");
  }
  myprintf("\n");
} 

int printQueue(PROC *queue, char *queueName) {
  PROC *p = 0;
  myprintf("%s = ", queueName);
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
  PROC *p = 0;

  p = get_proc(&freeList);
  //myprintf("kfork()\n");
  //myprintf("PROCESS RECEIVED:\n");
  //printProc(p);

  if(!p) {
    printf("no more PROC, kfork() failed\n");
    return 0;
  }

  p->status = READY;
  p->priority = 1;        //priority = 1 for all proc except P0
  p->ppid = running->pid; //parent = running
  p->parent = running;

  /* Initialize new proc's kstack[ ] */
  for (i = 1; i < 10; i++)          // saved CPU registers
    p->kstack[SSIZE - i] = 0; 

  p->kstack[SSIZE-1] = (int)body; // resume point = address of body()
  p->ksp = &(p->kstack[SSIZE-9]);   // proc saved sp
  enqueue(&readyQueue, p);        // enter p into readyQueue by priority

  return p;
}

int printProc(PROC *p) {
  if(p){
    myprintf("p:           %x\n", p);   
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

   running = freeList = readyQueue = sleepList = 0;

   myprintf("init ...");
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
       //printProc(p);
   }       
   running = &proc[0];
   running->status = READY;
   running->parent = &proc[0];
       
   
   proc[NPROC-1].next = NULL;             // all procs form a linear link list
   freeList = &proc[1];
   readyQueue = 0;

   myprintf("done\n");

 }

int body()
{ 
  PROC *child = 0;
  char c = '\0';
  
  myprintf("proc %d resumes to body()\n", running->pid);
  while(1){
    color = running->pid + 10;
    printQueues();
    myprintf("proc %d [%d ] running: parent=%d\n",running->pid,running->priority, running->parent->pid);
    myprintf("enter a char [s|q|f z|a|w] : ");
    c = getc();
    myprintf("%c\n", c);

    switch(c){
    case 's': 
      myprintf("proc %d tswitch()\n", running->pid);
      do_tswitch();
      myprintf("proc %d resumes\n", running->pid);
      break;
    case 'q': 
      do_exit();
      break;
    case 'f': 
      myprintf("proc %d kfork a child\n", running->pid);
      myprintf("child pid = %d\n", freeList->pid);
      do_kfork();
      break;
    case 'z' :
      do_sleep();
      break;
    case 'a' :
      do_wakeup();
      break;
    case 'w' :
      do_wait();
      break;
    }
  }
}

int main()
{
  myprintf("MTX starts in main()\n");
  init();
  myprintf("P%d running\n", running->pid);
  kfork();
  myprintf("P%d switch process\n", running->pid);
  tswitch();
  body();
}
