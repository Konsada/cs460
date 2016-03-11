#include "util.h"
#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED 
#define NULL 0
typedef enum {FREE, READY, RUNNING, STOPPED, SLEEP, ZOMBIE} STATUS;

typedef struct proc{
  struct proc *next;
  int    ksp;               // saved ksp when not running
  int    status;
  int    priority;
  int    pid;
  int    ppid;                // add pid for identify the proc
  struct proc   *parent;
  int event;
  int exitCode;
  int    kstack[SSIZE];      // proc stack area

}PROC;

int  procSize = sizeof(PROC);
int nproc = 0;
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList, *zombieList;    // define NPROC procs
extern int color;
char* string[30];
int  rflag = 0;

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
int do_exit();
int do_sleep();
int do_wakeup();
int do_wait();

void do_tswitch(){
  myprintf("proc %d tswitch()\n", running->pid);
  tswitch();
  myprintf("proc %d resumes\n", running->pid);
}

int do_kfork(){
  PROC *child = 0;

  myprintf("proc %d kfork a child\n", running->pid);
  child = kfork();
  if(child){
    myprintf("child pid = %d\n", child->pid);
    return child->pid;
  }
  else
    myprintf("Failed fork\n");
  return -1;
}

int do_stop() {
  myprintf("proc %d stop running\n", running->pid);
  running->status = STOPPED;
  tswitch();
  myprintf("proc %d resume from stop\n", running->pid);
}

int do_continue() {
  PROC *p;
  char *input;
  int pid;

  myprintf("enter pid to resume : ");
  gets(input);
  putc('\n');
  pid = getint(input);

  if(pid < 1 || pid >= NPROC){
    myprintf("invalid pid\n", pid);
    return 0;
  }
  p = &proc[pid];
  if(p->status == STOPPED){
    p->status = READY;
    enqueue(&readyQueue, p);
    return 1;
  }
  return 0;
}

int do_exit(){
  int exitValue = 0;
  char *input;

  if(running->pid == 1 && nproc > 2){
    myprintf("other procs still exist, P1 can't die yet!\n");
    return -1;
  }
  myprintf("Please enter an exitValue: ");
  gets(input);
  myprintf("\ngets(%s) complete\n", input);
  exitValue = getint(input);
  // myprintf("%d = getint(%x) complete\n", exitValue, input);
  myprintf("%d\n", exitValue);
  kexit(exitValue);
}

int do_sleep(){
  int event = 0;
  char *input;

  myprintf("Please enter an event value to sleep on: ");
  gets(input);
  putc('\n');
  //myprintf("\ngets(%s) complete\n", input);
  event = getint(input);
  //  myprintf("%d = getint(%s)\n", event, input);

  if(event)
    ksleep(event);
  else
    myprintf("Event not recognized!\n");
  return;
}

int do_wakeup(){
  int event = 0;
  char *input;

  myprintf("Please enter an event value to wake up: ");
  gets(input);
  putc('\n');
  //  myprintf("\ngets(%s) complete\n", input);
  event = getint(input);
  //  myprintf("%d = getint(%s)\n", event, input);

  if(event){
    kwakeup(event);
  }
  else{
    myprintf("Event not recognized!\n");
  }
}
int do_wait(){
  int pid, status;
  pid = kwait(&status);
  //  myprintf("waiting [pid: %d | status %d]\n", pid, status);
  if(pid<0){
    myprintf("proc %d wait error : no child\n", running->pid);
    return -1;
  }
  myprintf("proc %d found a ZOMBIE child %d exitValue=%d\n", running->pid, pid, status);
  return pid;
}

void ksleep(int event){
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //  reschedule();
  tswitch();
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
  /*
  for(i = 1; i < NPROC; i++) {
    p = &proc[i];
    //    myprintf("%d[status:%p,event:%d ] -> ", p->pid, p->status, p->event);
    //    getc();
    if(p->status == SLEEP && p->event == event) {
      p->event = 0;
      p->status = READY;
      enqueue(&readyQueue, p);
      myprintf("wakeup %d\n", p->pid);
    }
  }
  */
  sleepList = tempQueue;
  //  reschedule();
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
  /*
  myprintf("exitValue = %d\n", exitValue);
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
  /* wakeup parent and also P1 if necessary 
  kwakeup(running->parent);
  if(wakeupP1)
    kwakeup(&proc[1]);
  tswitch();
*/
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
  /*
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
    if(!hasChild){
    myprintf("proc %d wait error : no child\n",running->pid);
    return -1;
    }
    ksleep(running);
    }
  */
}
int reschedule() {
  PROC *p, *tempQueue = 0;

  while((p=dequeue(&readyQueue))){
    enqueue(&tempQueue, p);
  }
  readyQueue = tempQueue;

  rflag = 0;
  if(running->priority < readyQueue->priority)
    rflag = 1;

  /*  PROC *p, *tempReady = 0, *tempSleep = 0, *tempFree = 0, *tempZombie = 0;
      int i;
      for(i = 0; i < 9; i++) {
      p = &proc[i];
      switch(p->status){
      case FREE: enqueue(&tempFree, p);myprintf("%d enqueued as %p\n",p->pid, p->status);break;
      case READY : enqueue(&tempReady, p);myprintf("%d enqueued as %p\n",p->pid, p->status);break;
      case SLEEP : enqueue(&tempSleep, p);myprintf("%d enqueued as %p\n",p->pid, p->status);break;
      case ZOMBIE: enqueue(&tempZombie, p);myprintf("%d enqueued as %p\n",p->pid, p->status);break;
      }
      }
      freeList = tempFree;
      readyQueue = tempReady;
      sleepList = tempSleep;
      zombieList = tempZombie;
  */
}

int chpriority(int pid, int pri){
  PROC *p;
  int i, ok = 0, reQ = 0;

  if(pid == running->pid){
    running->priority = pri;

    if(pri < readyQueue->priority)
      rflag = 1;
    return 1;
  }
  for(i = 1; i < NPROC; i++){
    p = &proc[i];
    if(p->pid == pid && p->status != FREE){
      p->priority = pri;
      ok = 1;
      if(p->status == READY)
	reQ = 1;
    }
  }
  if(!ok){
    myprintf("chpriority failed\n");
    return -1;
  }
  if(reQ)
    reschedule(p);
}

int do_chpriority(){

  int pid, pri;
  char s[16];
  char *input;
  myprintf("input pid: ");
  gets(input);
  putc('\n');
  pid = getint(input);
  myprintf("input new priority ");
  gets(input);
  putc('\n');
  pri = getint(input);
  if(pri < 1) 
    pri = 1;
  chpriority(pid, pri);
}

/*
int reschedule() {
  PROC *p, *tempQ = 0;
  while((p = dequeue(&readyQueue))) {
    enqueue(&tempQ, p);
  }
  readyQueue = tempQ;
  rflag = 0;
  if(running->priority < readyQueue->priority)
    rflag = 1;
}
*/

int scheduler()
{
  if (running->status == RUNNING){
    running->status = READY;
    enqueue(&readyQueue, running);
  }
  running = dequeue(&readyQueue);
  running->status = RUNNING;
  color = 0x000A + (running->pid % 6);
  rflag = 0;
}

// e.g. get_proc(&freeList);
PROC *get_proc(PROC **list){
  PROC *p = *list;
  if(p)
    *list = p->next;
  return p;


  /*
 if(*list)
    return dequeue(&(*list));
  return 0;
  */
}
// e.g. put_proc(&freeList, p);
int put_proc(PROC **list, PROC *p) {

  p->status = FREE;
  p->next = *list;
  *list = p;

}

// linear queue with running proc in the highest priority 
int enqueue(PROC **queue, PROC *p) {
  PROC *temp = *queue;

  if(temp == 0 || p->priority > temp->priority){
    *queue = p;
    p->next = temp;
  }
  else{
    while(temp->next && p->priority <= temp->next->priority)
      temp = temp->next;
    p->next = temp->next;
    temp->next = p;
  }
  /*
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
    else {
      temp->next = p;
      p->next = NULL;
      return 0;
    }
  }
  while(temp->next && temp->next->priority >= p->priority) {
    temp = temp->next; 
  }
  p->next = temp->next;
  temp->next = p;
  return 0;
  */
}

// remove a PROC with the highest priority (the first one in queue)
// returns its pointer
PROC *dequeue (PROC **queue) {
  PROC *p = *queue;
  if(p)
    *queue = (*queue)->next;
  return p;
  /*
  if(*queue) {
    *queue = (*queue)->next;
  }
  p->next = NULL;
  */
  return p;
}
// dequeus a process with a certain id from the queue passed in
PROC *dequeuePid(PROC **queue, int pid) {
  PROC *p, *q = *queue, *r;

  p = q;
  while(p) {
    if(p->next && p->next->pid == pid) {
      r = p->next;
      p->next = r->next;
      return r;
    }
    p = p->next;
  }
  return NULL;
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
    myprintf("Queue is empty!\n");
    return 0;
  }

  myprintf("%s = ", queueName);

  while(p){
    myprintf("%d[%p]->", p->pid, p->status);
    //    myprintf("%d[p%d,e%d,x%d]->",p->pid, p->priority, p->event, p->exitCode);
    p = p->next;
  }
  /*
  if(&queue == &freeList){
    while(p){
      myprintf("%d->", p->pid);
      p = p->next;
    }
  }
  else if(queue == readyQueue){
    while(p){
      myprintf("[%d, %d]->", p->pid, p->priority);
      p = p->next;
    }
  }
  else if(queue == sleepList){
    while(p){
      myprintf("[%d, %d]->", p->pid, p->event);
      p = p->next;
    }
  }
  else if(queue == zombieList){
    while(p){
      myprintf("[%d, e%d]->", p->pid, p->exitCode);
      p = p->next;
    }
  }
  */
  myprintf("NULL\n");
  return 1;
}

void printProcs(){
  int i;
  for(i=0; i < NPROC; i++){
    myprintf("P[%d, %p]->", proc[i].pid, proc[i].status);
  }
  myprintf("NULL\n");
}
PROC *kfork() // create a child process, begin from body()
{
  int i;
  PROC *child = 0;

  child = get_proc(&freeList);

  if(!child) {
    printf("no more PROC, kfork() failed\n");
    return 0;
  }

  child->status = READY;
  child->priority = 1;        //priority = 1 for all proc except P0
  child->ppid = running->pid; //parent = running
  child->parent = running;

  child->kstack[SSIZE-1] = (int)body; // resume point = address of body()

  /* Initialize new proc's kstack[ ] */
  for (i = 1; i < 10; i++)          // saved CPU registers
    child->kstack[i] = 0; 

  child->ksp = &(child->kstack[SSIZE-9]);   // proc saved sp
  enqueue(&readyQueue, child);        // enter p into readyQueue by priority
  nproc++;
  return child;
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

   myprintf("intit...");

   running = freeList = readyQueue = sleepList = 0;

   myprintf("init ...");
   /* initialize all proc's */
   for (i=0; i<NPROC; i++){
       p = &proc[i];
       p->status = FREE;
       p->pid = i;                        // pid = 0,1,2,..NPROC-1
       p->priority = 0;
       //       p->ppid = 0;
       //       p->parent = 0;
       if(i < (NPROC - 1))
	 p->next = &proc[i+1];
       else
	 p->next = 0;
       if (i){                            // not for P0
          p->kstack[SSIZE-1] = (int)body; // entry address of body()
          for (j=2; j<10; j++)            // kstack[ ] high end entries = 0
               p->kstack[SSIZE-j] = 0;
          p->ksp = &(p->kstack[SSIZE-9]);
       }
   }       
   freeList = &proc[0];

   /********** create P0 as running **********/
   p = get_proc(&freeList);
   //   p->ppid = 0;
   p->status = RUNNING;
   running = p;
   nproc++;
   myprintf("done\n");

 }

int body()
{ 
  char c = '\0';
  //  color = running->pid + 10;
  myprintf("proc %d resumes to body()\n", running->pid);
  while(1){
    //    color = running->pid + 10;
    //  printQueues();
    myprintf("--------------------------------------------------\n");
    printQueue(freeList, "freeList");
    printQueue(readyQueue, "readyQueue");
    printQueue(sleepList, "sleepList");
    printQueue(zombieList, "zombieList");
    //  printProcs();
    myprintf("--------------------------------------------------\n");
    myprintf("proc %d [%d ] running: parent=%d\n",running->pid,running->priority, running->parent->pid);
    myprintf("enter a char [s|q|f z|a|w] : ");
    c = getc();
    myprintf("%c\n", c);

    switch(c){
    case 's': 
      do_tswitch();
      break;
    case 'q': 
      do_exit();
      break;
    case 'f': 
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
    case 't' : 
      do_stop();
      break;
    case 'c' :
      do_continue();
      break;
    case 'p' :
      do_chpriority();
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
  while(1){
    myprintf("P0 running\n");
    if(nproc==2 && proc[1].status != READY)
      myprintf("no runable process, system halts\n");
    while(!readyQueue);
    myprintf("P0 switch process\n");
    tswitch();
  }
}
