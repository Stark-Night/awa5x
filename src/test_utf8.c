#include "config.h"

#include <stdio.h>
#include <stdint.h>
#include "utf8.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     uint8_t buffer1[] = { 0xC3, 0x9C, 0x0 };
     uint8_t buffer2[] = { 0xE2, 0x9B, 0x8F, 0x0 };
     uint8_t buffer3[] = { 0xF0, 0x9D, 0x8C, 0x86, 0x0 };

     struct UTF8Result result = { 0 };

     result = utf8_decode(buffer1);
     abort_when(0xDC != result.point);
     abort_when(2 != result.bytes);

     result = utf8_decode(buffer2);
     abort_when(0x26CF != result.point);
     abort_when(3 != result.bytes);

     result = utf8_decode(buffer3);
     abort_when(0x1D306 != result.point);
     abort_when(4 != result.bytes);

     return 0;
}
