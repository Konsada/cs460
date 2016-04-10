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
      wakeup(&pp->room);               // wakeup any WRITER on pipe 
      return;
  }
  
  // YOUR CODE for the WRITE_PIPE case:

}
