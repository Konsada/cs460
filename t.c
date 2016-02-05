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

int body()
{ 
   char c;
   printf("proc %d resumes to body()\n", running->pid);
   while(1){
     color = running->pid + 7;
     printf("proc %d running : enter a key : ", running->pid);
     c = getc(); 
     printf("%c\n", c);
     tswitch();
   }
}

int init()
{
   PROC *p;
   int i, j;

   /* initialize all proc's */
   for (i=0; i<NPROC; i++){
       p = &proc[i];
       p->status = FREE;
       p->pid = i;                        // pid = 0,1,2,..NPROC-1
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
   
int scheduler()
{
   running = running->next;
}

main()
{
  printf("MTX starts in main()\n");
  init();
  kfork();
  while(1){
    if(readyQueue){
      myprintf("proc %d  running : enter a key : \n", running.pid);
      getc();
      tswitch();
    }
  }
}

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

int put_proc(PROC **list, PROC *p) {
  PROC *ptr;
  ptr = *list;
  while(ptr) {
  }
  ptr->next = p;
  return 0;
}

int enqueue(PROC **queue, PROC *p) {
  PROC *prev, *cur;
  prev = cur = *queue;
  while(cur) {
    
  }
  prev->next = p;
}
