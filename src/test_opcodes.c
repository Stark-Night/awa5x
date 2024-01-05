#include "config.h"

#include <stdio.h>
#include <string.h>
#include "opcodes.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     abort_when(0 != strcmp("BLO", opcode_name(BLO)));
     abort_when(0 == opcode_has_parameter(BLO));

     return 0;
}
