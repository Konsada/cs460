#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
char *table = "0123456789ABCDEF";
u16 BASE = 10;
#define stdout 1

void myprints(char *fmt) {
  while(*fmt){
    putc(*fmt);
    *fmt++;
  }
}

int rpu (u32 x) {

  char c;
  if(x) {
    c = table[x%BASE];
    rpu(x/BASE);
    putc(c);
  }
}
void myprintu(u32 x) {

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
  rpu(x);
  putc(' ');
}

void myprintx(u32 x) {
  BASE = 16;
  putc('0');
  putc('x');
  rpu((u32)x);
  BASE = 10;
}

void myprintl(u32 x) {
  if(x)
    rpu(x);
  else
    putc('0');
}

void myprintX(u32 x) {
  BASE = 16;
  putc('0');
  putc('x');
  rpu(x);
  BASE = 10;
}

void myprintf(char *fmt, ...){
  char *cp = fmt;
  u32 *ip = (u32 *)&fmt+1;
  u32 *up;
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
    case 'l' : myprintl((u32 *)*ip); break;
    case 'X' : myprintX((u32 *)*ip); break;
    }
    cp++; ip++;
  }
}
#endif
