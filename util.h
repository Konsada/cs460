#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

char *table = "0123456789ABCDEF";
u16 BASE = 10;
u16 nameCount = 0;
char *name[32];
char pathname[128];
char buf[1024];

void myprintf(char *fmt, ...);
int mystrcmp(char *s1, char *s2);
int strtok(char *path);
int mystrcpy(char dest[], char sorce[]);

int strtok(char *path){
  char *cp;
  cp = path;
  nameCount = 0;
  myprintf("tok(%s)\n", path);
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
  myprintf("pathname broken into %d names\n", nameCount);
}
int mystrcpy(char *dest, char *source){
  int i = 0;
  myprintf("mystrcpy(%s)\n", source);
  while(source[i]) dest[i] = source[i++];
  dest[i] = 0;
  return 0;
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
