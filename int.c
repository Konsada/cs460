#include "type.h"
#include "util.h"

/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/
#define PA 13
#define PB 14
#define PC 15
#define PD 16
#define AX  8

/****************** syscall handler in C ***************************/
int kcinth()
{
  u16 segment, offset;
  int a,b,c,d, r;
  int i;

  segment = running->uss;
  offset = running->usp;

  a = get_word(segment, offset + 2*PA);
  b = get_word(segment, offset + 2*PB);
  c = get_word(segment, offset + 2*PC);
  d = get_word(segment, offset + 2*PD);

  switch(a){
  case 0 : r = kgetpid();        break;
  case 1 : r = kps();            break;
  case 2 : r = kchname(b);       break;
  case 3 : r = kkfork();         break;
  case 4 : r = ktswitch();       break;
  case 5 : r = kkwait(b);        break;
  case 6 : r = kkexit(b);        break;
  case 7 : r = fork();           break;
  case 8 : r = exec(b);          break;

/****** these are YOUR pipe functions ************/
   case 30 : r = kpipe(&b);         break; 
   case 31 : r = read_pipe(b,c,d);  break;
   case 32 : r = write_pipe(b,c,d); break;
   case 33 : r = close_pipe(b);     break;
   case 34 : r = pfd();             break;
  /**************** end of pipe functions ***********/

  case 99: do_exit(b);            break;
  default: 
    printf("invalid syscall # : %c\n", a);
    getc();
  }
  //  printf("put_word(r, segment, offset + 2*AX) : put_word(%d, %x, %x + %d)\n", r, segment, offset, (2*AX));
  put_word(r, segment, offset - 2*AX);

}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
  return running->pid;
}

int kps()
{
  //    WRITE C code to print PROC information
  printf("in kernel\n");
  return do_ps();
}

int kchname(char *name)
{
  printf("in kernel\n");
  return do_chname(name);
}

int kkfork()
{
  printf("in kernel\n");
  kmode();
}

int ktswitch()
{
  tswitch();
}

int kkwait(int *status)
{
  printf("proc %d calling kwait\n", running->pid);
  return do_wait(status);
}

int kkexit(int value)
{
  printf("in kernel\n");
  return do_exit(value);
}


