#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned int u16;
char *table = "0123456789ABCDEF";
u16 BASE = 10;

void myprintf(char *fmt, ...);
int mystrcmp(char *s1, char *s2);

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


char *gets(){
  char string[256];  
  char c;
  int i = 0;
  c = getc();
  for(i = 0; i < 256 && c != '\r' && c; i++){
    if(c == 8){ // backspace
      if(i > 0)
	i--;
      else
	i = 0;
    }
    else{
      string[i] = c;
    }
    c = getc(); 
  }
  myprintf("string too long!\n");
}


int getint(char *fmt) {
  int i = 0, len = myStrLen(fmt), sum = 0; 
  char c = fmt;

  for(;i < len;i++) {
    if(fmt[i] < 48 && fmt[i] > 57) {
      myprintf("Failed to read integer!\n");
      return;
    }
    else {
      sum = sum + pow(10,len-i)*fmt[i];
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
  if(fmt)
    return 1 + myStrLen(fmt + 1);
  else
    return 0;
}


void myprints(char *fmt) {
  if(fmt[0]) {
    putc(fmt);
    myprints(fmt+1);
  }
  else
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
    case 's' : myprints((char *)*ip); break;
    case 'u' : myprintu(*ip); break;
    case 'd' : myprintd(*ip); break;
    case 'x' : myprintx(*ip); break;
    }
    cp++; ip++;
  }
}
#endif
