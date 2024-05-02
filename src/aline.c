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

#include <stdlib.h>
#include "aline.h"

struct ALine
aline_start(struct ALine aline) {
     if (NULL != aline.items) {
          return aline;
     }

     aline.items = malloc(1024 * sizeof(struct ALineItem));
     aline.size = 1024;
     aline.capacity = 0;

     if (NULL == aline.items) {
          abort();
     }

     return aline;
}

struct ALine
aline_end(struct ALine aline) {
     if (NULL == aline.items) {
          return aline;
     }

     free(aline.items);
     aline.items = NULL;
     aline.size = 0;
     aline.capacity = 0;

     return aline;
}

struct ALine
aline_track(struct ALine aline, struct ALineItem item) {
     if (aline.capacity + 1 >= aline.size) {
          size_t newsize = aline.size * 2;
          if (newsize <= aline.size) {
               abort();
          }

          aline.items = realloc(aline.items, newsize);
          aline.size = newsize;

          if (NULL == aline.items) {
               abort();
          }
     }

     aline.items[aline.capacity].code = item.code;
     aline.items[aline.capacity].parameter = item.parameter;
     aline.items[aline.capacity].address = item.address;
     aline.items[aline.capacity].flags = item.flags;

     aline.capacity = aline.capacity + 1;

     return aline;
}

struct ALine
aline_reset(struct ALine aline) {
     aline.capacity = 0;

     return aline;
}

struct ALine
aline_change_flags_at(struct ALine aline, size_t index, uint16_t flags) {
     if (index >= aline.capacity) {
          // this is a programming bug; an assert would've been
          // better, probably, but I prefer being more drastic
          abort();
     }

     aline.items[index].flags = flags;

     return aline;
}

struct ALine
aline_add_flags_at(struct ALine aline, size_t index, uint16_t flags) {
     if (index >= aline.capacity) {
          // this is a programming bug; an assert would've been
          // better, probably, but I prefer being more drastic
          abort();
     }

     aline.items[index].flags = aline.items[index].flags | flags;

     return aline;
}

struct ALine
aline_remove_flags_at(struct ALine aline, size_t index, uint16_t flags) {
     if (index >= aline.capacity) {
          // this is a programming bug; an assert would've been
          // better, probably, but I prefer being more drastic
          abort();
     }

     aline.items[index].flags = aline.items[index].flags & ~flags;

     return aline;
}
