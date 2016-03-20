int copyImage(u16 seg1, u16 seg2, u16 size){
  int i; u16 temp;

  for(i = 0; i < size; i++){
    temp = get_word(seg1, 2*i);
    put_word(temp, seg2, 2*i);
  }
}

int goUmode();

int fork(){
  PROC *child; u16 segment;

  child = kfork(0);
  if(child == 0){
    return -1;
  }

  segment = (child->pid + 1) * 0x1000;
  myprintf("pid = %d segment = %x\n", child->pid, segment);

  copyImage(running->uss, segment, 32*1024);

  child->uss = segment;
  child->usp = running->usp;

  put_word(segment, segment, child->usp);
  put_word(segment, segment, child->usp+2);
  put_word(0, segment, child->usp+2*8);
  put_word(segment, segment, child->usp+2*10);
  nproc++;

  printQueue(readyQueue, "readyQueue");
  myprintf("Proc %d forked a child %d at segment = %x\n", running->pid, child->pid, segment);
  return child->pid;
}

extern int loader();

int exec(char *filename){
  int i, length;
  u16 segment, offset, high;
  char line[64], *cp, c;

  char file[32], f2[16], *cq;

  segment = running->uss;

  cp = line;
  while((*cp = get_byte(segment, filename)) != 0){
    filename++;cp++;
  }

  segment = (running->pid + 1)*0x1000;

  cp = line;
  cq = f2;

  while(*cp == ' ')cp++;

  while(*cp != ' ' && *cp != 0)
    (*cq)++ = (*cp)++; // may not work
  *cq = 0;

  if(f2[0] == 0) return -1;

  strcpy(file, "/bin/");
  strcat(file, f2);

  myprintf("Proc %d exec to %s in segment %x\n", running->pid, file, segment);

  if(!loading(file, segment)){
    printf("exec %s failed\n", file);
    return -1;
  }

  length = strlen(line) + 1;
  if(length % 2) length++;

  offset = -length;

  for (i = 0; i < len; i++)
    put_byte(line[i], segment, offset + i);

  high = offset - 2;

  put_word(offset, segment, high);

  for(i = 1; i <= 12; i++)
    put_word(0, segment, high-2*i);

  running->uss = segment;
  running->usp = high-2*12;

  put_word(running->uss, segment, high-2*12);  // uDS = uSS
  put_word(running->uss, segment, high-2*11);  // uES = uSS
  put_word(0, segment, high-2*3);              // uPC = 0
  put_word(segment, segment, high-2*2);        // uCS = segment  
  put_word(0x0200, segment, high-2);           // flag
}
