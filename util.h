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
    case 's' : prints((char *)*ip); break;
    case 'u' : printu(*ip); break;
    case 'd' : printd(*ip); break;
    case 'x' : printx(*ip); break;
    case 'l' : printl((u32 *)*ip); break;
    case 'X' : printX((u32 *)*ip); break;
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

int rpu (u32 x) {

  char c;
  if(x) {
    c = table[x%BASE];
    rpu(x/BASE);
    putchar(c);
  }
}
void printu(u32 x) {

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

void printx(u32 x) {
  BASE = 16;
  putchar('0');
  putchar('x');
  rpu((u32)x);
  BASE = 10;
}

void printl(u32 x) {
  if(x)
    rpu(x);
  else
    putchar('0');
}

void printX(u32 x) {
  BASE = 16;
  putchar('0');
  putchar('x');
  rpu(x);
  BASE = 10;
}
#endif
