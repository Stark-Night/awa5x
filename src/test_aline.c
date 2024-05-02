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
#include "aline.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     struct ALine aline = { 0 };
     struct ALineItem item = { 0 };

     aline = aline_start(aline);

     item.code = 0;
     item.parameter = 1;
     item.flags = 0;
     aline = aline_track(aline, item);

     item.code = 1;
     item.parameter = 0;
     item.flags = 0;
     aline = aline_track(aline, item);

     abort_when(0 != aline.items[0].code);
     abort_when(1 != aline.items[0].parameter);
     abort_when(0 != aline.items[0].flags);

     abort_when(1 != aline.items[1].code);
     abort_when(0 != aline.items[1].parameter);
     abort_when(0 != aline.items[1].flags);

     aline = aline_change_flags_at(aline, 1, 5);
     abort_when(5 != aline.items[1].flags);

     aline = aline_change_flags_at(aline, 1, 0);
     aline = aline_add_flags_at(aline, 1, 1);
     abort_when(1 != aline.items[1].flags);
     aline = aline_add_flags_at(aline, 1, 2);
     abort_when(3 != aline.items[1].flags);
     aline = aline_remove_flags_at(aline, 1, 1);
     abort_when(2 != aline.items[1].flags);

     aline = aline_end(aline);

     return 0;
}
