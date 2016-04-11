show_pipe(PIPE *p)
{
   int i, j;
   printf("------------ PIPE CONTENETS ------------\n");     
   // print pipe information
   printf("\n----------------------------------------\n");
}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
  // print running process' opened file descriptors
}
//============================================================


int read_pipe(int fd, char *buf, int n)
{
  // your code for read_pipe() 
  // check for data
  //read as much as it needs(up to pipe size)
  //return number of bytes read
  //if pipe has no data but still has writers, pipe waits and wakes up writers
  return 0; //pipe has now data and no writers
}

int write_pipe(int fd, char *buf, int n)
{
  // your code for write_pipe()
  // check for room
  // write as much as needed or until pipe is full
  // if pipe is full but has readers, writer waits for room and wakes up readers
  return 0; //pipe has no more readers // broken pipe error
}

int kpipe(int pd[2])
{
  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
  /*
  -  create a pipe
  -  fork a child process
   */
  PIPE *p;
  OFT *oft0;
  OFT *oft1;
  int i;
  //search for open pipe
  for(i = 0; i < NPIPE; i++){
    if(!pipe[i].busy)
      break;
  }
  //all pipes are busy
  if(i == NPIPE) {
    printf("all pipes are busy!\n");
    return -1;
  }
  //allocate newly found pipe
  pipe[i].busy = 1;
  p = &pipe[i];
  p->head = 0;
  p->tail = 0;
  p->data = 0;
  p->room = PSIZE;

  //search for reader oft
  for(i = 0; i < NOFT; i++)
    if(!oft[i].refCount)
      break;
  
  oft0 = &oft[i];

  for(;i < NOFT; i++)
    if(!oft[i].refCount)
      break;
  
  oft1 = &oft[i];

  if(i == NOFT){
    printf("no available file descriptors!\n");
    p->busy = 0;
    return -1;
  }

  //allocate reader oft
  oft0->refCount++;
  oft1->refCount++;
  oft0->mode = READ_PIPE;
  oft1->mode = WRITE_PIPE;
  oft0->pipe_ptr = p;
  oft1->pipe_ptr = p;
  p->nreader = 1;
  p->nwriter = 1;

  //allocate ofts to running proc
  for (i = 0; i < NFD; i+= 2){
    if(running->fd[i] == 0 && running->fd[i+1] == 0){
      running->fd[i] = oft0;
      running->fd[i+1] = oft1;
    }
  }
  if(i == NFD){
    printf("all fd on proc %u are occupied!\n", running->pid);
    p->busy = 0;
    oft0->refCount = 0;
    oft1->refCount = 0;
    p->nreader = 0;
    p->nwriter = 0;
    return -1;
  }
  //allocate return values
  pd[0] = i;
  pd[1] = i+1;
  printf("do_pipe : file descriptors = [%d %d]\n", pd[0], pd[1]);
  return 0;
}

int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  running->fd[fd] = 0;                 // clear fd[fd] entry 

  if (op->mode == READ_PIPE){
      pp = op->pipe_ptr;
      pp->nreader--;                   // dec n reader by 1

      if (--op->refCount == 0){        // last reader
	if (pp->nwriter <= 0){         // no more writers
	     pp->busy = 0;             // free the pipe   
             return;
        }
      }
      kwakeup(&pp->room);               // wakeup any WRITER on pipe 
      return;
  }
  
  // YOUR CODE for the WRITE_PIPE case:

}
