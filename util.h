#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
char *table = "0123456789ABCDEF";
u16 BASE = 10;

void myprintf(char *fmt, ...){
  char *cp = fmt;
  u32 *ip = (u32 *)&fmt+1;
  u32 *up;
  while(*cp){
    if(*cp != '%'){
      putchar(*cp);
      if(*cp == '\n')
	putchar('\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp) {
    case 'c' : putchar((char)*ip); break;
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

void myprints(char *fmt) {
  while(*fmt){
    putchar(*fmt);
    *fmt++;
  }
}

int rpu (u32 x) {

  char c;
  if(x) {
    c = table[x%BASE];
    rpu(x/BASE);
    putchar(c);
  }
}
void myprintu(u32 x) {

  if(x)
    rpu(x);
  else
    putchar('0');
  putchar(' ');
}

void myprintd(int x) {

  if(x<0){
    putchar('-');
    x = x*(-1);
  }
  rpu(x);
  putchar(' ');
}

void myprintx(u32 x) {
  BASE = 16;
  putchar('0');
  putchar('x');
  rpu((u32)x);
  BASE = 10;
}

void myprintl(u32 x) {
  if(x)
    rpu(x);
  else
    putchar('0');
}

void myprintX(u32 x) {
  BASE = 16;
  putchar('0');
  putchar('x');
  rpu(x);
  BASE = 10;
}
#endif
