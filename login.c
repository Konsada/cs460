#include "ucode.c"

int in, out, err, fd, n;
int uid, gid;
char buf[1024];
char *tty;
char username[64], password[64], *item[8], line[64];
char *cp, *cq, *cpp, *cqq;
main(int argc, char *argv[]){
  int i = 0;
  tty = argv[1];

  close(0);
  close(1);
  close(2);

  in = open(tty, O_RDONLY);
  out = open(tty, O_WRONLY);
  err = open(tty, O_WRONLY);

  settty(tty); // switch terminal

  signal(2, 1);

  printf("Keon's login: open %s as stdin, stdout, stderr\n", tty);

  while(1){
    // ask user for login credentials
    printf("++++++++++++++++++++\n");
    printf("login:");
    gets(username);
    printf("password:");
    gets(password);
    printf("++++++++++++++++++++\n");

    fd = open("/etc/passwd", 0); // open credentials file

    if(fd < 0){  // no file found
      printf("no /etc/passwd file\n"); 
      exit(1);
    }

    n = read(fd, buf, 2048); // copy file into buffer
    buf[n] = 0;

    cp = cq = buf;
    //format buffer before scanning
    while(cp < &buf[n]){ // one line at a time
      while(*cq != '\n'){
	if(*cq == ' ') *cq = '_';
	if(*cq == ':') *cq = ' ';
	cq++;
      }
      *cq = 0;
      strcpy(line, cp);
      cpp = cqq = line;
      i = 0;
      //scan buffer
      while(*cqq){
	if(*cqq == ' '){
	  *cqq = 0;
	  item[i] = cpp;
	  i++;
	  cqq++; cpp = cqq;
	  continue;
	}
	cqq++;
      }
      // compare username and password entered with what is found in /etc/passwd
      if((strcmp(username, item[0]) == 0) && (strcmp(password, item[1]) == 0)){
	printf("Welcome! %s\n", username);
	printf("$HOME = %s ", item[5]);
	gid = atoi(item[2]);
	uid = atoi(item[3]);

	chdir(item[5]);

	printf("uid now %d", uid);
	chuid(uid, 0);
	printf("exec to /bin/sh... \n");
	close(fd);
	exec("sh");
      }
      cq++;
      cp = cq;
    }
    // incorrect username/password combination entered
    printf("login failed, try again...\n");
    close(fd);
  }
}
