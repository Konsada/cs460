#include "type.h"
#include "util.h"

PROC *kfork(char *filename) // create a child process, begin from body()
{
  int i, segment,pid;
  PROC *child = 0;

  child = get_proc(&freeList);

  if(!child) {
    printf("no more PROC, kfork() failed\n");
    return 0;
  }
  pid = child->pid;

  segment = (pid + 1)*1000;

  child->status = READY;
  child->priority = 1;        //priority = 1 for all proc except P0
  child->ppid = running->pid;//parent = running
  child->parent = running;
  child->uss = segment;
  child->usp = segment - 24;

  child->kstack[SSIZE-1] = (int)body; // resume point = address of body()

  /* Initialize new proc's kstack[ ] */
  for (i = 1; i < 10; i++)          // saved CPU registers
    child->kstack[i] = 0; 

  child->ksp = &(child->kstack[SSIZE-9]);   // proc saved sp
  enqueue(&readyQueue, child);        // enter p into readyQueue by priority
  nproc++;

  if(filename){
    makeUimage("/bin/u1", child);
  }

  return child;
}

int makeUimage(char *filename, PROC *p){
  u16 i, segment;
  i=0;

  segment = (p->pid + 1)*0x01000;

  myprintf("loading file %s onto segment %u with proc %d\n", filename, segment, p->pid);
  load(filename, segment);

  while(pathList[i])printf("path[%d]:%s\n",i,pathList[i++]);
  for(i = 1; i <= 12; i++){
    put_word(0, segment, -2*i);
  }
  put_word(0x0200, segment, -2*1);
  put_word(segment, segment, -2*2);
  put_word(segment, segment, -2*11);
  put_word(segment, segment, -2*12);

  p->usp = -2*12;
  p->uss = segment;
  return 0;
}

void do_tswitch(){
  myprintf("proc %d tswitch()\n", running->pid);
  tswitch();
  myprintf("proc %d resumes\n", running->pid);
}

int do_kfork(){
  PROC *child = 0;

  myprintf("proc %d kfork a child\n", running->pid);
  child = kfork("/bin/u1");
  if(child){
    myprintf("child pid = %d\n", child->pid);
    return child->pid;
  }
  else
    myprintf("Failed fork\n");
  return -1;
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
  //  myprintf("\ngets(%s) complete\n", input);
  exitValue = getint(input);
  // myprintf("%d = getint(%x) complete\n", exitValue, input);
  myprintf("\n%d\n", exitValue);
  kexit(exitValue);
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
    myprintf("enter a char [s|f|w|q|u] : ");
    c = getc();
    myprintf("%c\n", c);

    switch(c){
    case 's': 
      do_tswitch();break;
    case 'f': 
      do_kfork();break;
    case 'w' :
      do_wait();break;
    case 'q': 
      do_exit();break;
    case 'u' : 
      goUmode();break;
    }
  }
}

int kmode(){
  body();
}
char *statusStrings[ ]  = {"FREE   ", "READY  ", "RUNNING", "STOPPED", "SLEEP  ", "ZOMBIE ", 0};
int do_ps(){
  int i, j;
  char *p, *q, buf[16];
  printf("do_ps()\n");
  buf[15] = 0;

  myprintf("========================================\n");
  myprintf("  name        status     pid     ppid  \n");//2 name 8 status 5 pid 5 ppid
  myprintf("----------------------------------------\n");

  for(i = 0; i < NPROC; i++){
    p = proc[i].name;
    for(j = 0; j < 15; j++){
      if(*p)
	buf[j] = *(p++);
      else
	buf[j] = ' ';
    }
    myprintf("%s", buf);
    myprintf("%s", statusStrings[proc[i].status]);
    myprintf("   %d       ", proc[i].pid);
    myprintf("%d\n", proc[i].ppid);
  }
  myprintf("----------------------------------------\n");
  return 0;
}

int do_chname(char *newName){
  char buf[64];
  char *cp = buf;
  int count = 0;

  while(count < 32){
    *cp = get_byte(running->uss, newName);
    if(*cp == 0) break;
    cp++; newName++; count++;
  }
  buf[31] = 0;

  myprintf("changing name of proc %d to %s\n", running->pid, buf);
  strcpy(running->name, buf);
  myprintf("done\n");
}

int do_kkfork(){
  PROC *p = kfork("/bin/u1");
  if(!p){
    return -1;
  }
  return p->pid;
}
