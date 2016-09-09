#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

char *table = "0123456789ABCDEF";
u16 BASE = 10;

void printf(char *fmt, ...){
  char *cp = fmt;
  u16 *ip = (u16 *)&fmt+1;
  u16 *up;
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
    case 's' : prints((char *)*ip); break;
    case 'u' : printu(*ip); break;
    case 'd' : printd(*ip); break;
    case 'x' : printx(*ip); break;
    }
    cp++; ip++;
  }
}

void prints(char *fmt) {
  while(*fmt){
    putchar(*fmt);
    *fmt++;
  }
}

int rpu (u16 x) {

  char c;
  if(x) {
    c = table[x%BASE];
    rpu(x/BASE);
    putchar(c);
  }
}
void printu(u16 x) {

  if(x)
    rpu(x);
  else
    putchar('0');
  putchar(' ');
}

void printd(int x) {

  if(x<0){
    putchar('-');
    x = x*(-1);
  }
  rpu(x);
  putchar(' ');
}

void printx(u16 x) {
  BASE = 16;
  putchar('0');
  putchar('x');
  rpu((u16)x);
  BASE = 10;
}

void printl(u16 x) {
  if(x)
    rpu(x);
  else
    putchar('0');
}

void printX(u16 x) {
  BASE = 16;
  putchar('0');
  putchar('x');
  rpu(x);
  BASE = 10;
}
#endif
