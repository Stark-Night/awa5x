/* awa5x - Extended AWA5.0
   Copyright © 2024 Starknights

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>. */

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
