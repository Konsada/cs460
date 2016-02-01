#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void main(int argc, char **argv, char **env){
  myprintf("This is a test of char %c\n", 'a');
  myprintf("This is a test of string %s\n", "test");
  myprintf("This is a test of uint %u\n", 24);
  myprintf("This is a test of int %d\n", -24);
  myprintf("This is a test of x %x\n", 334568);
}
