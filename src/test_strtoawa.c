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
#include "strtoawa.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     int32_t n = 0;
     char *tail = NULL;

     n = strtoawa("123", &tail);
     abort_when(123 != n || NULL == tail || '\0' != tail[0]);

     n = strtoawa("~123", &tail);
     abort_when(-123 != n || NULL == tail || '\0' != tail[0]);

     n = strtoawa("123X", &tail);
     abort_when(123 != n || NULL == tail || 'X' != tail[0]);

     n = strtoawa("12147483647", &tail);
     abort_when(INT32_MAX != n);

     n = strtoawa("~12147483647", &tail);
     abort_when(INT32_MIN != n);

     n = strtoawa("-123", &tail);
     abort_when(NULL == tail || '-' != tail[0]);

     return 0;
}
