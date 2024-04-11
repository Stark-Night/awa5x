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
#include <string.h>
#include "gap.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     struct GapBuffer buffer = { 0 };

     buffer = gap_append(buffer, "test", 4);
     buffer = gap_move(buffer, 0);
     buffer = gap_append(buffer, "1", 1);
     buffer = gap_move(buffer, 5);

     abort_when(0 != memcmp("1test", buffer.bytes, 5));

     buffer = gap_shrink(buffer);
     abort_when(NULL != buffer.bytes && 0 != buffer.size);

     return 0;
}
