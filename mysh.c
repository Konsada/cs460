#include "ucode.c"

int userUid;
char buf[1024], temp[1024], cmdList[64];
char *cp, *cq, *cpp, *cqq;
char *cmds[8], cmdline[1024];
int numCmds;

main(int argc, char *argv[]){
  int i = 0;
  int status;
  signal(2, 1);

  printf("Keon's sh!\n");
  printf("enter ? for help menu\n");

  while(1){
    printf("Keon's sh :");
    numCmds = 0;

    if(gets(buf) == 0)exit(0);
    
    cp = buf;

    while(*cp == ' '){ // bypass rid of leading empty space
      cp++;
    }
    cq = cp;

    // ___command1 | command2  |  command3________^
    //                                         cq
    while(*cq){     
      cq++; // move pointer to end
    }
    cq--;
    while(*cq == ' '){ // removes trailing blanks
      *cq = 0;
      cq--;
    }
    if(strcmp(cp, "") == 0) continue;
    if(*cp == 4) exit(0);    
    // command1 | command2 | command3^000000
    // cp                            cq
    strcpy(temp, cp);
    strcpy(cmdline, temp);
    cp = cmdline;
    while(*cp){
      if(*cp == '|') numCmds++;
      cp++;
    }
    numCmds++;
    cp = cmdline;
    printf("%d commands found on proc %d!\n", numCmds, getpid());

    /**************************************************
         NEED TO READ COMMANDS INTO CMDS[] // done

      cp = cmdline;

      for(i = 0; i < numCmds; i++){
	printf("parsing pipes\n");
	while(*cp && *cp != ' ')cp++;
	cq = cp;
	while(*cp){
	  if(*cp == '|'){
	    printf("pipe found!\n");
	    break;
	  }
	  cp++;
	}
	if(*cp == '|'){
	*cp = 0;
	  cp++;
	}
	strcpy(cmds[i],cq);
      }
      strcpy(cmdline,temp);
      cp = cmdline;
    **************************************************/
    numCmds = 0;
    eatpat(temp, cmdList, cmds, &numCmds);

    printf("numCmds: %d\n", numCmds);

    if(strcmp(cmds[0], "cd") == 0){                       //  cd
      printf("cd found!\n");
      if(cmds[1] == 0){
	chdir("/");
      }
      else{
	chdir(cmds[1]);	    
	getcwd(buf);
	printf("%s\n", buf);
      }
      continue;
    }
    if(strcmp(cmds[0], "pwd") == 0){                  // pwd
      printf("pwd found\n");
      getcwd(buf);
      printf("%s", buf);
      continue;
    }
    if(strcmp(cmds[0], "logout") == 0 || strcmp(cmds[0], "exit") == 0){              // logout
      printf("loging out of uid: %d\n", getuid());
      chdir("/");
      exit(0);
    }
    if(strcmp(cmds[0], "su") == 0){                                           // su
      switchUser();
    }
    if(strcmp(cmds[0], "echo") == 0){
      for(i = 1; i < numCmds; i++){
	printf("%s ", cmds[i]);
      }
      printf("\n");
      continue;
    }
    if(strcmp(cmds[0], "?") == 0 || strcmp(cmds[0], "help") == 0){
      printf("help selected\n");
      help(); continue;
    }
      
    printf("Parent pid %d forks a kitten ", getpid());
    pid = fork();

    if(pid){
      printf(" parent sh %d is waiting for kitten\n", getpid());
      pid = wait(&status);
      continue;
    }
    else{
      printf("kitten sh %d is running cmd: %s\n", getpid(), cmdline);
      do_pipe(cmdline, 0);
    }    
  }
}

int switchUser(){
  if(getuid() == 0){
    printf("already root user! Silly goose...\n");
  }
  else{
    printf("Enter root password:");
    gets(buf);
    if(strcmp(buf,"12345")){
      printf("success!\n");
      chuid(0,0);
      printf("UID: %d\nGID: %d\n",getuid(),0);
    }
    else{
      printf("Nope! Wrong password friend...\n");
    }
  }
}

int do_pipe(char *cmdLine, int *rightpd)
{
  char *tail;
  int leftpd[2], hasPipe, pid;

  printf("do_pipe(cmdLine:%s, rightpd:%d)\n", cmdLine, rightpd);

  if (rightpd){ // if has a pipe passed in, as WRITER on pipe pd:  
    close(rightpd[0]); 
    close(1);
    dup(rightpd[1]); 
    close(rightpd[1]);
  }

  hasPipe = scan(cmdLine, &tail); 

  if (hasPipe){
    if(pipe(leftpd) < 0){
      printf("proc %d pipe creation failed miserably...\n", getpid());
      exit(1);
    }
    pid = fork();
    if(pid < 0){
      printf("proc %d failed to fork a kitten\n", getpid());
      exit(1);
    }
    if (pid){ // parent
      close(leftpd[1]); 
      close(0);
      dup(leftpd[0]); 
      close(leftpd[0]);

      do_command(tail);
    }
    else // recursively call do_pipe
      do_pipe(cmdLine, leftpd);
  }
  else{ // no pipes, just do command
    do_command(cmdLine);  
  }
} 

int help(){
  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  printf("! ls     cd     pwd     cat   cd     mv     ps   !\n");
  printf("! mkdir  rmdir  creat   rm    chmod  more   grep !\n");
  printf("! l2u    (I/O and Pipe) :  >  >>     <      |    !\n");
  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

char headCmd[64], execLine[64];

int do_command(char *cmdLine)
{
  //  scan cmdLine for I/O redirection symbols;
  //  do I/O redirections;
  char *p, *q, filename[64], *cmds[16], arguments[64];
  int i, j, savedIndex, numCmds;
  p = cmdLine;
  printf("do_command(cmdLine:%s)\n", cmdLine);

  /*while(*p){
    if(*p == '>'){
      if(*(p+1) == '>'){
	// >> redirection
	printf("Append ouput to ");
	p++;
	q = p;
	while(*q == ' ') q++;
	if(*q == 0){
	  printf("INVALID APPEND COMMAND FOOL!\n");
	  exit(1);
	}
	close(1);
	for(i = 0; *(q+i) && *(q+i) != ' '; i++);

	if(*(q+i)) *(q+i) = 0;
	strncpy(filename, q, i);
	printf("%s\n", filename);
	open(filename, O_WRONLY|O_CREAT|O_APPEND);
	break;
      }
      else{
	// > redirection
	printf("Redirect output to ");
	p++;
	q = p;
	while(*q == ' ') q++;
	if(*q == 0){
	  printf("INVALID OUTPUT REDIRECT COMMAND FOOL!\n");
	  exit(2);
	}	
	close(1);
	for(i = 0; *(q+i) && *(q+i) != ' '; i++);

	if(*(q+i)) *(q+i) = 0;
	strncpy(filename, q, i);
	printf("%s\n", filename);
	open(filename, O_WRONLY|O_CREAT);
	break;
      }
    }
    else if(*p == '<'){
      // < redirection
      printf("Redirect input from ");
      p++;      
      q = p;     
      while(*q == ' ') q++;
      if(*q == 0){
	printf("INVALID INPUT REDIRECT COMMAND FOOL!\n");
	exit(3);
      }
      close(0);
      for(i = 0; *(q+i) && *(q+i) != ' '; i++);

      if(*(q+i)) *(q+i) = 0;
      strncpy(filename, q, i);
      if(open(filename, 0) < 0){
	printf("open(filename:%s, 0) failed!\n", filename);
	printf("File for input redirect doesn't exist!\n");
	exit(4);
      }
      break;
    }
    // no redirection yet!
    p++;
  }
  //  head = cmdLine BEFORE redirections
  strcpy(headCmd, cmdLine);
  p = headCmd;
  while(*p && *p == ' ')p++;
  strcpy(headCmd,p);
  while(*p){
    if(*p == '<' || *p == '>' || *p == ' '){
      *p = 0;
      break;
    }
    p++;
  } 
  */
  eatpat(cmdLine, headCmd, cmds, &numCmds);
  savedIndex = numCmds;

  for(i = 0; i < numCmds; i++){
    if(strcmp(cmds[i], "<") == 0){
      printf("redirect input found!\n");
      if(savedIndex > i) savedIndex = i;
      if(cmds[i+1] == 0){
	printf("input redirection failed!\n");
	exit(1);
      }
      close(0);
      if(open(cmds[i+1], 0) < 0){
	printf("open(filename:%s, 0) failed!\n", cmds[i+1]);
	printf("filename doesn't exist\n");
	exit(2);
      }
      break;
    }
  }
  for(i = 0; i < numCmds; i++){
    if(strcmp(cmds[i], ">") == 0){
      printf("redirect output found!\n");
      if(savedIndex > i) savedIndex = i;
      if(cmds[i+1] == 0){
	printf("ouput redirection failed!\n");
	exit(3);
      }
      close(1);
      printf("open(filename:%s, O_WRONLY|O_CREAT)\n", cmds[i+1]);
      open(cmds[i+1], O_WRONLY|O_CREAT);
      break;
    }
  }
  for(i = 0; i < numCmds; i++){
    if(strcmp(cmds[i], ">>") == 0){
      printf("append output found!\n");
      if(savedIndex > i) savedIndex = i;
      if(cmds[i+1] == 0){
	printf("append redirection failed!\n");
	exit(4);
      }
      close(1);
      printf("open(filename:%s, O_WRONLY|O_CREAT|O_APPEND)\n", cmds[i+1]);
      open(cmds[i+1], O_WRONLY|O_CREAT|O_APPEND);
      break;
    }
  }

  i = 0;
  while(cmds[i]){
    printf("cmds[%d]: %s\n",i,cmds[i]);
    i++;
  }

  for(i = 0; i < 64; i++)execLine[i] = 0;

  strcpy(execLine, cmds[0]);
  printf("execLine: %s, cmds[0]: %s\n", execLine, cmds[0]);
  printf("numCmds:%d\n",numCmds);
  printf("savedIndex: %d\n", savedIndex);

  for(j = 1; j < savedIndex; j++){
    strcat(execLine, " ");
    strcat(execLine, cmds[j]);
    printf("execLine: %s, cmds[%d]\n", execLine,j);
  }
    //    printf("Only 2 cmds\n");
    //    for(i = 0; execLine[i]; i++);
    //    headCmd[i] = ' ';
    //    i++;
    //    strcpy(execLine[i]," ");
    //    i++;
    //    strcpy(execLine[i], cmds[1]);
    //    headCmd[i] = 0;

    //    for(j = 0; cmds[1][j] != 0;j++){
      //      headCmd[i+j] = cmds[1][j];  
    //    }
    //    headCmd[i+j] = 0;
  
  printf("exec(execLine:%s)\n", execLine);
  exec(execLine);
  getc();
}

int scan(char *head, char **tail)
{
  char *p, *q;

  p = head; *tail = 0;

  while(*p)         // scan to buf end line
    p++;    

  while (p != head && *p != '|') // scan backward until |
    p--;

  if (p == head)     // did not see any |, so head=buf
    return 0;

  *p = 0;           // change | to NULL 
  p++;              // move p right by 1
  while(*p == ' ')  // skip over any blanks
    p++;

  *tail = p;        // change tail pointer to p

  return 1;       // head points at buf; return head
}
