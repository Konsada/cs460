#include "ucode.c"
int color;
main()
{ 
  char name[64]; int pid, cmd;

  while(1){
    pid = getpid();
    color = 0x0C;
    printf("u2.c\n");       
    printf("----------------------------------------------\n");
    printf("THIS IS PROC %d in U MODE: SEGMENT=%x\n",getpid(), getcs());
    show_menu();
    printf("COMMAND ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    printf("%d\n", cmd);
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : exit();     break;
           case 7 : fork();     break;
           case 8 : exec();     break;
// ADD fork, exec cases 
//
           default: invalid(name); break;
    }
  }
}



