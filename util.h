#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned int u16;
char *table = "0123456789ABCDEF";
u16 BASE = 10;
u16 nameCount = 0;
char *name[32];
char path[128];
char buf[1024];

void myprintf(char *fmt, ...);
int mystrcmp(char *s1, char *s2);
int strtok(char *path);
int search(MINODE *mip, char *name, int dev);
u16 search(INODE *ip, char *name);

u16 search(INODE *ip, char *name) {
  int i; char c; DIR *dp;

  for(i = 0; i < 12; i++) {
    if((u16)ip->i_block[i]) {
      getblk((u16)ip->i_block[i], b2);
      dp = (DIR *)buf;

      while ((char *)dp < &buf[1024]) {
	c = dp->name[dp->name_len];
	dp->name[dp->name_len] = 0;

	if(strcmp(dp->name, name) == 0) {
	  return ((u16)dp->inode);
	}
	dp->name[dp->name_len] = c;
	dp = (char *)dp + dp->rec_len;
      }
    }
  }
  return 0;
}

int search(MINODE *mip, char *name, int dev) {
  int i, *iblk, *diblk;
  char buf[BLOCK_SIZE], *cp, ibuf[BLOCK_SIZE], dibuf[BLOCK_SIZE], *temp;
  char *bp, *dbp;
  if(DEBUG) printf("DEBUG: search()\n");
  get_block(dev, mip->inode.i_block[0], buf); 
  // search direct blocks
  if(DEBUG) printf("DEBUG: mip->inode.i_blocks: %u\n", mip->inode.i_blocks);
  for(i = 0; i < mip->inode.i_block[i]; i++) {
    get_block(dev, mip->inode.i_block[i], buf);    
    cp = buf;
    dp = (DIR *)buf;
    while(cp < buf + BLOCK_SIZE) {
      temp = (char *)calloc(257,0);
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      if(DEBUG) printf("DEBUG: ...Searching...\n");
      if(DEBUG) printf("DEBUG: dp->name: %s dp->inode: %u dp->rec_len: %u\n", temp, dp->inode, dp->rec_len);
      if(!strncmp(dp->name, name, dp->name_len)){
	temp = (char *)calloc(64,0);
	strncpy(temp,dp->name,dp->name_len);
	if(DEBUG) printf("DEBUG: %s found, returning ino: %d.\n", temp,dp->inode);
	return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  /***********************************************
   !!!NEED TO IMPLEMENT INDIRECT SEARCH AS WELL!!!
  ***********************************************/
  // search single indirect blocks
  if(mip->inode.i_blocks >= 12) {
    get_block(dev, mip->inode.i_block[12], ibuf);
    bp = ibuf;
  if(DEBUG) printf("DEBUG: implementing single indirect search for %s\n", name);
    //while bp doesn't pass the indirectbuffer end point
    while(bp < ibuf + BLOCK_SIZE) {    
      get_block(dev, *bp, buf); // get this block into buf
      cp = buf;
      dp = (DIR *)cp;
      while(cp < buf + BLOCK_SIZE) {
	if(!strncmp(dp->name, name, dp->name_len))return dp->inode;
	cp += dp->rec_len;
	dp = (DIR *)cp;
      }
      bp += sizeof(u32); // increment 1024 / 256 = 4bytes || 32bits
    }
  }
  if(mip->inode.i_blocks >= 13){
    // search double indirect blocks
    get_block(dev, mip->inode.i_block[13], dibuf);
    dbp = dibuf;
    if(DEBUG) printf("DEBUG: implementing double indirect search for %s\n", name);
 
    // single indirect block traversal
    while(dbp < dibuf + BLOCK_SIZE) {
      get_block(dev, *dbp, ibuf);
      bp = ibuf;
      // double indirect block traversal
      while(bp < ibuf + BLOCK_SIZE) {
	get_block(dev, *bp, buf);
	cp = buf;
	dp = (DIR *)cp;
	// block traversal
	while(cp < buf + BLOCK_SIZE) {
	  if(!strncmp(dp->name, name, dp->name_len))return dp->inode;
	  cp += dp->rec_len;
	  dp = (DIR *)cp;
	}
	bp += sizeof(32);
      }
      dbp += sizeof(u32);
    }
  }
  return 0;
}

int strtok(char *path){
  char *cp;
  cp = path;
  nameCount = 0;

  while(*cp != 0) {
    while(*cp == '/') *cp++ = 0;
    if(*cp != 0)
      name[nameCount++] = cp;
    while(*cp != '/' && *cp != 0) cp++;
    if(*cp != 0)
      *cp = 0;
    else
      break;
    cp++;
  }
}

// < 0 if s1 is less than s2 and > 0 if s1 is less than s2
int mystrcmp(char *s1, char *s2){
  int i = 0;
  while(s1[i] && s2[i]){
    if(!(s1[i] - s2[i])) {
      i++;
    }
    else
      return s1[i] - s2[i];
  }
  return 0;
}


char *myfscanf(){
  char string[256];  
  char c = '\0';
  int i = 0;

  myprintf("gets()\n");

  for(i = 0; i < 256 && (c != '\r'); i++){
    c = getc();
    myprintf("%c entered\n", c);
    if(c == 8){ // backspace
      if(i > 0)
	i--;
      else
	i = 0;
    }
    // character entered
    else{
      string[i] = c;
      string[i+1] = '\0';
      myprintf("string: %s\n", string);
    } 
    if(c == '\r');
    {
      string[i] = '\n';
      return string;
    }
  }
  myprintf("string too long!\n");
  return 0;
}


int gets(char *s) {
  while((*s = getc()) != '\r') {
    putc(*s++);
  }
  *s=0;
}

int getint(char *fmt) {
  int i = 0, len = 0, sum = 0; 
  char c = 0;
  myprintf("fmt: %d\n", &fmt);
  len = myStrLen(fmt);

  myprintf("%s has a length of %d\n", fmt, len);
  for(i = 0;i < len;i++) {
    if(fmt[i] < 48 && fmt[i] > 57) {
      myprintf("Failed to read integer!\n");
      return;
    }
    else {
      sum = sum + pow(10,(len-1)-i)*(fmt[i] - 48);
    }
  }
  return sum;
}


int pow(int base, int power) {
  if(base == 0)
    return 0;
  else if(power == 0)
    return 1;
  else
    return base * pow(base, power-1);
}


int myStrLen(char *fmt) {
  int i = 0;
  while(*fmt++) {
    myprintf("%c[%d],",*fmt, i);
    i++;
  }
  putc('\n');
  return i;
}


void myprints(char *fmt) {
  char *c;
  c = fmt;
  while(*c) {
    putc(*c);
    c++;
  }
  return;
}

int rpu (u16 x) {

  char c;
  if(x) {
    c = table[x%BASE];
    rpu(x/BASE);
    putc(c);
  }
}


void myprintu(u16 x) {

  if(x)
    rpu(x);
  else
    putc('0');
  putc(' ');
}


void myprintd(int x) {

  if(x<0){
    putc('-');
    x = x*(-1);
  }
  if(x)
    rpu(x);
  else
    putc('0');
}


void myprintx(u16 x) {
  BASE = 16;
  putc('0');
  putc('x');
  rpu((u16)x);
  BASE = 10;
}

void myprintp(u16 x) {
  switch(x){
  case 0 : myprintf("FREE"); break;
  case 1 : myprintf("READY"); break;
  case 2 : myprintf("SLEEP"); break;
  case 3 : myprintf("BLOCK"); break;
  case 4 : myprintf("ZOMBIE"); break;
  default : myprintf(" ");
  }
}
void printl(u16 x) {
  if(x)
    rpu(x);
  else
    putc('0');
}

void printX(u16 x) {
  BASE = 16;
  putc('0');
  putc('x');
  rpu(x);
  BASE = 10;

}

void myprintf(char *fmt, ...){
  char *cp = fmt;
  u16 *ip = (u16 *)&fmt+1;
  u16 *up;

  while(*cp){
    if(*cp != '%'){
      putc(*cp);
      if(*cp == '\n')
	putc('\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp) {
    case 'c' : putc((char)*ip); break;
    case 's' : myprints(*ip); break;
    case 'u' : myprintu(*ip); break;
    case 'd' : myprintd(*ip); break;
    case 'x' : myprintx(*ip); break;
    case 'p' : myprintp(*ip); break;
    }
    cp++; ip++;
  }
}

#endif
