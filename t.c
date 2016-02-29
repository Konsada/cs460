#include "util.h"
#include "type.h"

#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define NULL 0

enum STATUS {FREE, READY, SLEEP, BLOCK, ZOMBIE};


typedef struct proc{
  struct proc *next;
  int    ksp;               // saved ksp when not running
  int    uss;
  int    usp;
  int    status;
  int    priority;
  int    pid;
  int    ppid;                // add pid for identify the proc
  struct proc   *parent;
  int    kstack[SSIZE];      // proc stack area
  int event;
  int exitCode;
}PROC;

typedef struct ext2_group_desc {
  u32 bg_block_bitmap;
  u32 bg_inode_bitmap;
  u32 bg_inode_table;
  u16 bg_free_blocks_count;
  u16 bg_free_inodes_count;
  u16 bg_used_dirs_count;
  u16 bg_pad;
  u32 bg_reserved[3];
} GD;

typedef struct ext2_inode {
  u16 i_mode;
  u16 i_uid;
  u32 i_size;
  u32 i_atime;
  u32 i_ctime;
  u32 i_mtime;
  u32 i_dtime;
  u16 i_gid;
  u16 i_links_count;
  u32 i_blocks;
  u32 i_flags;
  u32 reserved;
  u32 i_block[15];
  u32 i_pad[7];
}INODE;

typedef struct ext2_dir_entry_2 {
  u32 inode;
  u16 rec_len;
  u8 name_len;
  u8 file_type;
  char name[255];
} DIR;    // need this for new version of e2fs

typedef struct head{
  u32 ID_space;              // 0x04100301: combined I&D or 0x04200301: separate I&D
  u32 magic_number;          // 0x00000020
  u32 tsize;                 // code section size in bytes
  u32 dsize;                 // initialized data section size in bytes
  u32 bsize;                 // bss section size in bytes
  u32 zero;                  // 0
  u32 total_size;            // total memory size, including heap
  u32 symbolTable_size;      // only if symbol table is present
}HEADER;

u16 tsize, dsize, bsize, totalSize;
int  procSize = sizeof(PROC);
u16 nproc = 0;

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList, *zombieList;    // define NPROC procs
extern int color;
char* string[30];
int  rflag = 0;

GD    *gp;
//SUPER *sp;
INODE *ip;
DIR   *dp; 

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
int get_block(u16 blk, char *buf);
int move(u16 segment);
int clearbss(u16);
int load(char *filename, u16 segment);
u16 search(INODE *ip, char *name);


int makeUimage(char *filename, PROC *p) {
  u16 i, segment;

  //segment is p's space in memory
  segment = (p->pid + 1)*0x1000;
  load(filename, segment);

  //clear ustack in segment

  for(i = 1; i <= 12; i++) {
    put_word(0, segment, (-2*i));
  }

  // write in flag(1), uCS(2), uES(11), uDS(12)

  put_word(0x0200, segment, (-2*1));
  put_word(segment, segment, (-2*2));
  put_word(segment, segment, (-2*11));
  put_word(segment, segment, (-2*12));

  // uSP and uss needs to be set
  p->usp = -2*12;
  p->uss = segment;
  return 0;
}

int load(char *filename, u16 segment){
  u32 *temp;
  u16 i, curInode;
  HEADER *tempH;

  mystrcpy(path, filename);
  strtok(path);

  get_block(2, buf);
  gp = (GD*)buf;
  get_block((u16)gp->bg_inode_table, buf);
  ip = (INODE*)buf + 1;

  for(i = 0; i < nameCount; i++) {
    curInode = search (ip, name[i]);
    if(!curInode) {
      myprintf("\n%s not found!\n", name[i]);
      return -1;
    }
    curInode--;
    get_block(gp->bg_inode_table + (curInode / 8), buf);
    ip = (INODE *)buf + (curInode % 8);
  }

  //save header info
  get_block(ip->i_block[0], buf);
  tempH = (HEADER *)buf;
  tsize = tempH->tsize;
  dsize = tempH->dsize;
  bsize = tempH->bsize;
  totalSize = tempH->total_size;

  if(ip->i_block[12])
    get_block((u16)ip->i_block[12], buf);

  setes(segment);

  for(i = 0; i < 12 && ip->i_block[i]; i++){
    get_block((u16)ip->i_block[i], 0); // load i_block[i]
    inces(); // increment ES by 1K
  }

  if((u16)ip->i_block[12]) {
    temp = (u32*)buf;
    while(*temp){
      get_block((u16)*temp, 0);
      inces();
      temp++;
    }
  }

  move(segment);

  clearbss(segment);
  setes(0x1000);
  myprintf(" done\n");
  return 1;
}

u16 search(INODE *ip, char *name) {
  int i; char c; DIR *dp;

  for(i = 0; i < 12; i++) {
    if((u16)ip->i_block[i]) {
      get_block((u16)ip->i_block[i], buf);
      dp = (DIR *)buf;

      while ((char *)dp < &buf[1024]) {
	c = dp->name[dp->name_len];
	dp->name[dp->name_len] = 0;

	if(strcmp(dp->name, name) == 0) {
	  return ((u16)dp->inode);
	}
	dp->name[dp->name_len] = c;
	dp = (char *)dp + dp->rec_len;
      }
    }
  }
  return 0;
}

int move(u16 segment) {
  u16 i, j;

  for(i = 0; i < tsize+dsize; i+=2) {
    j = get_word(segment+2, i);
    put_word(j, segment, i);
  }
}

int clearbss(u16 segment) {
  u16 i, j, seg, remainder;

  remainder = (tsize + dsize) % 16;

  for( i = 0; i < remainder; i++) {
    put_byte(0, (segment + (tsize + dsize)/16), i);
  }
  for(j = 0; j < bsize; j++) {
    put_byte(0, (segment + (tsize + dsize)/16), j + i);
  }
}
int get_block(u16 blk, char *buf){
  // Convert blk into (C,H,S) format by Mailman to suit disk geometry
  //      CYL           HEAD            SECTOR
  diskr( blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
}

int makeUserImage(char *filename, PROC *p) {
  u16 i, segment;

  segment = (p->pid+1)*0x1000;
  load(filename,segment);

  for(i = 1; i <= 12; i++) {
    put_word(0, segment, -2*i);
  }

  put_word(0x200, segment, -2*1);
  put_word(segment, segment, -2*2);
  put_word(segment, segment, -2*11);
  put_word(segment, segment, -2*12);

  p->usp = -2*12;
  p->uss = segment;
  return 0;
}
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
  int exitValue = 0;
  char *input;

  myprintf("Please enter an exitValue: ");
  gets(input);
  myprintf("\ngets(%s) complete\n", input);
  exitValue = getint(input);
  myprintf("%d = getint(%x) complete\n", exitValue, input);

  if(exitValue){
    kexit(exitValue);
  }
  else
    myprintf("Exit value not recognized!\n");
  return;
}

void do_sleep(){
  int event = 0;
  char *input;

  myprintf("Please enter an event value to sleep on: ");
  gets(input);
  myprintf("\ngets(%s) complete\n", input);
  event = getint(input);
  myprintf("%d = getint(%s)\n", event, input);

  if(event)
    ksleep(event);
  else
    myprintf("Event not recognized!\n");
  return;
}

void do_wakeup(){
  int event = 0;
  char *input;

  myprintf("Please enter an event value to wake up: ");
  gets(input);
  myprintf("\ngets(%s) complete\n", input);
  event = getint(input);
  myprintf("%d = getint(%s)\n", event, input);

  if(event){
    kwakeup(event);
  }
  else{
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
  enqueue(&sleepList, running);
  reschedule();
  tswitch();
}
void kwakeup(int event){
  int i = 0;
  PROC *p;
  for(i = 1; i < NPROC; i++) {
    p = &proc[i];
    myprintf("%d[status:%p,event:%d ] -> ", p->pid, p->status, p->event);
    getc();
    if(p->status == SLEEP && p->event == event) {
      p->event = 0;
      p->status = READY;
      enqueue(&readyQueue, p);
      myprintf("pid: %d enqueued!\n", p->pid);
    }
  }
  reschedule();
}

int kexit(int exitValue){
  int i, wakeupP1 = 0;
  PROC *p;
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
int scheduler()
{
  if (running->status == READY){
    enqueue(&readyQueue, running);
  }
  running = dequeue(&readyQueue);
  rflag = 0;
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

int kexec(char *umodeFilePointer) {
  int i, length = 0;
  char filename[64], *cp = filename;
  u16 segment = running->uss;

  while((*cp++ = get_byte(running->uss, umodeFilePointer++)) && length++ < 64);
  if(!load(filename, segment))
    return -1;

  for(i = 1; i <= 12; i++) 
    put_word(0, segment, (-2*i));

  running->usp = -24;

  put_word(segment, segment, (-2*12));  // uDS = segment
  put_word(segment, segment, (-2*11));  // uES = segment
  put_word(segment, segment, (-2*2));   // uCS = segment, uPC = 0
  put_word(0x0200, segment, (-2*1));    // Umode flag=0x0200
}

PROC *kfork(char *filename) // create a child process, begin from body()
{
  int i;
  PROC *p = 0;
  u16 segment;

  p = get_proc(&freeList);

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
  printQueue(readyQueue, "readyQueue");
  nproc++;
  segment = (p->pid+1)*0x1000;

  if(filename){
    segment = 0x1000*(p->pid+1);
    makeUserImage(filename, p);
  }
  printf("PROC %d forked a child %d in segment=%x\n", running->pid, p->pid, segment);
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
  color = running->pid + 10;
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
