#ifndef UTIL_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

void myprintf(char *fmt, ...){
  char *cp = fmt;
  u16 *ip = (u16 *)&fmt+1;

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
    case 'c' : putc(*ip); break;
    case 's' : prints(ip); break;
    case 'u' : printu(ip); break;
    case 'd' : printd(ip); break;
    case 'x' : printx(ip); break;
    case 'l' : printl((u32 *)ip++); break;
    case 'X' : printX((u32 *)ip++); break;
    }
    cp++; ip++;
  }
}

void prints(u16 *ip) {
  while(*ip++){
    putc(*ip);
    ip++;
  }
}

void printu(u16 *ip) {

}

void printd(u16 *ip) {

}

void printx(u16 *ip) {

}

void printl(u32 *ip) {

}

void printX(u32 *ip) {

}
#endif
