#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned int u16;
char *table = "0123456789ABCDEF";
u16 BASE = 10;



void myprints(char *fmt) {
  while(*fmt){
    putc(*fmt);
    *fmt++;
  }
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
