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
#include "abyss.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     struct Abyss abyss = { 0 };
     struct Bubble bubble = { 0 };

     abyss = abyss_expand(abyss);
     abort_when(NULL == abyss.bubbles);
     abort_when(NULL == abyss.free);
     abort_when(abyss.free != abyss.bubbles);
     abort_when(abyss.free->next != abyss.bubbles + 1);
     abyss = abyss_drop(abyss);
     abort_when(NULL != abyss.bubbles);

     bubble = bubble_wrap(8);
     abort_when(8 != bubble.value);

     abyss = abyss_push(abyss, bubble);
     abort_when(NULL == abyss.bubbles);
     abort_when(0 == abyss.size);
     abort_when(0 == abyss.used);
     abort_when(abyss.head == abyss.free);
     abort_when(abyss.head != abyss.bubbles);
     abort_when(abyss.free != abyss.bubbles + 1);

     abort_when(bubble.value != (abyss_top(abyss)).value);

     abyss = abyss_pop(abyss);
     abort_when(0 != abyss.used);
     abort_when(NULL != abyss.head);

     abyss = abyss_drop(abyss);
     abort_when(NULL != abyss.bubbles);

     bubble.value = 0;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 1;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 2;
     abyss = abyss_push(abyss, bubble);

     abort_when(abyss.head != abyss.bubbles + 2);

     abyss = abyss_move(abyss, 2);
     abort_when(abyss.head != abyss.bubbles + 1);
     abort_when(1 != abyss.head->value);
     abort_when(2 != abyss.head->next->next->value);

     abyss = abyss_drop(abyss);

     bubble.value = 0;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 1;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_join(abyss, 2);
     abort_when(abyss.bubbles + 2 != abyss.head);
     abort_when(abyss.bubbles + 1 != abyss.head->head);
     abort_when(NULL != abyss.head->next);
     abort_when(abyss.bubbles + 0 != abyss.head->head->next);

     bubble.value = 3;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_join(abyss, 2);
     abort_when(3 != abyss.head->head->value);
     abort_when(NULL == abyss.head->head->next);
     abort_when(NULL == abyss.head->head->next->head);
     abyss = abyss_pop(abyss);
     abort_when(3 != abyss.head->value);
     abort_when(NULL == abyss.head->next->head);

     abyss = abyss_drop(abyss);

     bubble.value = 0;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 1;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_merge(abyss);
     abort_when(NULL == abyss.head->head);
     abort_when(1 != abyss.head->head->value);

     bubble.value = 2;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 3;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_merge(abyss);
     abyss = abyss_merge(abyss);
     abort_when(3 != abyss.head->head->value);

     abyss = abyss_drop(abyss);

     bubble.value = 10;
     abyss = abyss_push(abyss, bubble);
     int ex = abyss.size;
     abyss = abyss_expand(abyss);
     bubble.value = 23;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_expand(abyss);
     bubble.value = 88;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_expand(abyss);
     abort_when(88 != abyss.head->value
                || 23 != abyss.head->next->value
                || 10 != abyss.head->next->next->value);
     abort_when(ex * 8 != abyss.size);

     abyss = abyss_drop(abyss);

     int64_t testbuf[] = {
          INT64_MIN,
          13,
          24,
          INT64_MIN,
          33,
          18,
          12,
          INT64_MAX,
          87,
          99,
          INT64_MAX,
     };

     abyss = abyss_external(abyss, testbuf, 11);
     abort_when(NULL == abyss.head);
     abort_when(NULL == abyss.head->head);
     abort_when(13 != abyss.head->head->value);
     abort_when(24 != abyss.head->head->next->value);
     abort_when(NULL == abyss.head->head->next->next->head);
     abort_when(33 != abyss.head->head->next->next->head->value);
     abort_when(87 != abyss.head->head->next->next->next->value);
     abort_when(99 != abyss.head->head->next->next->next->next->value);

     abyss = abyss_drop(abyss);

     return 0;
}
