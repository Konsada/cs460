#include "ucode.c"

int userUid;


int help(){
  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  printf("! ls     cd     pwd     cat   cd     mv     ps   !\n");
  printf("! mkdir  rmdir  creat   rm    chmod  more   grep !\n");
  printf("! l2u    (I/O and Pipe) :  >  >>     <      |    !\n");
  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

char buf[1024], temp[1024];
char *cp, *cq, *cpp, *cqq;
char *cmds[8], cmdline[1024];
int numCmds;
main(int argc, char *argv[]){
  int i = 0;
  signal(2, 1);

  printf("Keon's sh!\n");
  printf("enter ? for help menu\n");

  while(1){
    gets(buf);
    if(buf != 0){
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
      if(*cp == 4){
	exit(0);
      }
      // command1 | command2 | command3^000000
      // cp                            cq
      strcpy(tbuf, cp);
      strcpy(cmdline, tbuf);
      cp = cmdline;
      while(*cp){
	if(*cp == '|') numCmds++;
	cp++;
      }
      numCmds++;
      if(numCmds == 1){ // only one cmd
	if(strcmp(cmds[0],"cd") == 0){                       //  cd
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
	else if(strcmp(cmds[0],"pwd") == 0){                  // pwd
	  getcwd(buf);
	  printf("%s", buf);
	  continue;
	}
	else if(strcmp(cmds[0], "logout") == 0 || strcmp(cmds[0], "exit") == 0){              // logout
	  printf("loging out of uid: %d\n", getuid());
	  chdir("/");
	  exit(0);
	}
	else if(strcmp(cmds[0], "su") == 0){                  // su
	  switchUser();
	}
	else if(strcmp(chmds[0], "echo") == 0){
	  printf("%s\n", cmds[1]);
	  continue;
	}
	else if(strcmp(chmds[0], "?") == 0 || strcmp(chmds[0], "help") == 0){
	  help(); continue;
	}
      }      
    }
    else{
	printf("parent sh %d forks a kitten \n", getpid());

	pid = fork();

	if(pid){
	  printf("parent sh %d waits for kitten\n", getpid());
	  pid = wait(&status);
	  continue;
	}
	else{
	  printf("kitten sh %d \n", getpid());
	  setcolor(0x000C);

	}
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

int do_pipe(char *cmdLine, int *pd)
{
  if (pd){ // if has a pipe passed in, as WRITER on pipe pd:  
    close(pd[0]); dup2(pd[1],1); close(pd[1]);
  }
  // divide cmdLine into head, tail by rightmost pipe symbol
  hasPipe = scan(cmdLine, head, tail); 
  if (hasPipe){
    create a pipe lpd;
    pid = fork();
    if (pid){ // parent
      as READER on lpd: 
	close(lpd[1]); dup2(lpd[0],0); close(lpd[0]);
      do_command(tail);
    }
    else
      do_pipe(head, lpd);
  }
  else
    do_command(cmdLine);  
} 

int do_command(char *cmdLine)
{
  scan cmdLine for I/O redirection symbols;
  do I/O redirections;
  head = cmdLine BEFORE redirections
    exec(head);
}

int scan(char *cmdLine, char *head, char *tail)
{
  // divide cmdLine into head and tail by rightmost | symbol
  // cmdLine = cmd1 | cmd2 |...|cmdn-1 |  cmdn 
  //           |<------- head --------->| tail |; return 1;
 P // cmdLine = cmd1 ==> head=cmd1, tail=null;     return 0;  
}
