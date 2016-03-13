
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

  case 99: kkexit(b);            break;
  default: printf("invalid syscall # : %d\n", a); 
  }

  put_word(r, segment, offset + 2*AX);

}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
  return running->pid;
}

int kps()
{
  //    WRITE C code to print PROC information
  return do_ps();
}

int kchname(char *name)
{
  //    WRITE C CODE to change running's name string;
  return do_chname(name);
}

int kkfork()
{
  //  use you kfork() in kernel;
  //  return child pid or -1 to Umode!!!
  return do_kkfork();
}

int ktswitch()
{
  return tswitch();
}

int kkwait(int *status)
{
  return kwait(&status);
}

int kkexit(int value)
{
  return kexit(value);
}
