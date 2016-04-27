#include "ucode.c"
int console, cdserver, s0, s1;
int main (){
  int in, out, console1, console2, console3;     // file descriptors for terminal I/O
  in = open("/dev/tty0", O_RDONLY);
  out = open("/dev/tty0", O_WRONLY); // for display to console
  printf("INIT : fork a login proc on console\n");

  /*  console = 0;

  console = fork();

  if(console){
    exec("login /dev/tty0");
  }  
  console = 0;
  console = fork();
  if(console){
    exec("login /dev/ttyS0");
  }
  console = 0;
  console = fork();
  if(console){
    exec("login /dev/ttyS1");
  }
  else 
    parent();
  */
  console = fork();

  if(console){
    printf("login on s0 terminal\n");
    s0 = fork();
    if(s0){
      printf("login on s1 terminal\n");
      s1 = fork();
      if(s1){
	parent();
      }
      else{
	exec("login /dev/ttyS1");
      }
      parent();
    }
    else{
      exec("login /dev/ttyS0");
    }
    parent();
  }
  else{
    exec("login /dev/tty0");
  }
}

int parent(){
  int pid, status;
  while(1){
    printf("INIT : wait for ZOMBIE kitten\n");
    pid = wait(&status);
    if (pid==console){
      printf("INIT: forks a new console login\n");
      console = fork();
      if (console)
	continue;
      else
	exec("login /dev/tty0");
    }
    printf("INIT: I just buried an orphan kitten proc %d\n", pid);
  }
}
