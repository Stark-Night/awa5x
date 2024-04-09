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
#include <string.h>
#include "grow.h"

static struct GrowBuffer *
grow_buffer(struct GrowBuffer *buffer, size_t requested) {
     if (NULL == buffer->bytes) {
          buffer->bytes = malloc(1024);
          buffer->size = 1024;
          buffer->capacity = 0;

          if (NULL == buffer->bytes) {
               abort();
          }
     }

     if (buffer->capacity + requested < buffer->size) {
          return buffer;
     }

     size_t newsize = buffer->size;
     while (buffer->capacity + requested > newsize) {
          newsize = newsize * 2;
     }

     if (newsize <= buffer->size) {
          abort();
     }

     buffer->bytes = realloc(buffer->bytes, newsize);
     buffer->size = newsize;

     if (NULL == buffer->bytes) {
          abort();
     }

     return buffer;
}

struct GrowBuffer
append_buffer(struct GrowBuffer buffer, void *data, size_t size) {
     grow_buffer(&buffer, size);

     memcpy(buffer.bytes + buffer.capacity, data, size);
     buffer.capacity = buffer.capacity + size;

     return buffer;
}

struct GrowBuffer
reset_buffer(struct GrowBuffer buffer) {
     buffer.bytes[0] = '\0';
     buffer.capacity = 0;
     return buffer;
}

struct GrowBuffer
shrink_buffer(struct GrowBuffer buffer) {
     free(buffer.bytes);

     buffer.bytes = NULL;
     buffer.size = 0;
     buffer.capacity = 0;

     return buffer;
}
