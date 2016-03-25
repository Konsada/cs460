// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", "fork", "exec", 0};

#define LEN 64

int show_menu()
{
   printf("************************ Menu ************************\n");
   printf("*  ps  chname  kmode  switch  wait  exit  fork  exec *\n");
   printf("******************************************************\n");
}

int find_cmd(char *name)
{
  int i;
  for(i = 0; cmd[i]; i++){
    if(!strcmp(name, cmd[i]))
      return i;
  }
  return -1;
}

int getpid()
{
  //  printf("entering the kernel to get pid...\n");
   return syscall(0,0,0);
}

int ps()
{
  // printf("entering the kernel to print processes...\n");
   return syscall(1, 0, 0);
}

int chname()
{
    char s[64];
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

int kmode() {
  printf("kmode : enter Kmode via INT 80\n");
  printf("proc %d going K mode ....\n", getpid());
  syscall(3, 0, 0);
  printf("proc %d back from Kernel\n", getpid());
}

int fork()
{   
  int child, pid;
  //  pid = getpid();
  //  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(7, 0, 0, 0);
  if(!child)
    printf("kfork failed!\n");
  else
    printf("proc %d kforked a child %d\n", getpid(), child);
}    

int kswitch()
{
  printf("proc %d entering Kernel to switch process\n", getpid());
    return syscall(4,0,0);
    printf("proc %d back form Kernel\n", getpid());
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
} 


int geti()
{
  // return an input integer
  char input[16];
  return atoi(gets(input));

}


int exit()
{
   int exitValue;
   char *input;
   printf("enter an exitValue: ");
   //   gets(input);
   //   exitValue = getint(input);
   exitValue = geti();
   printf("exitvalue=%d\n", exitValue);
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _kexit(exitValue);
}

int _kexit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}

int exec(){
  int r; 
  char filename[32];
  printf("enter exec filename : ");
  gets(filename);
  r = syscall(8, filename, 0, 0);
  printf("exec failed\n");
}

int vfork(){
  return syscall(9, 0, 0, 0);
}
