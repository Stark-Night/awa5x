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
#include "gap.h"

static struct GapBuffer *
grow_buffer(struct GapBuffer *buffer, size_t requested) {
     if (NULL == buffer->bytes) {
          buffer->bytes = malloc(1024);
          buffer->size = 1024;
          buffer->start = 0;
          buffer->end = buffer->size / 2;

          if (NULL == buffer->bytes) {
               abort();
          }
     }

     if (requested < buffer->size) {
          return buffer;
     }

     size_t newsize = buffer->size;
     while (requested > newsize) {
          newsize = newsize * 2;
     }

     if (newsize < buffer->size) {
          abort();
     }

     size_t end = buffer->end;
     size_t endsize = buffer->size - buffer->end;

     buffer->bytes = realloc(buffer->bytes, newsize);
     buffer->size = newsize;
     buffer->end = buffer->size - endsize;

     if (NULL == buffer->bytes) {
          abort();
     }

     memmove(buffer->bytes + buffer->end, buffer->bytes + end, endsize);

     return buffer;
}

struct GapBuffer
gap_append(struct GapBuffer buffer, void *data, size_t size) {
     while (buffer.start + size >= buffer.end) {
          grow_buffer(&buffer, size);
     }

     memcpy(buffer.bytes + buffer.start, data, size);
     buffer.start = buffer.start + size;

     return buffer;
}

struct GapBuffer
gap_move(struct GapBuffer buffer, size_t where) {
     if (0 == buffer.size) {
          grow_buffer(&buffer, 1024);
     }

     if (where > buffer.size - buffer.end - buffer.start) {
          where = buffer.size - buffer.end - buffer.start;
     }

     if (where < buffer.start) {
          size_t diff = buffer.start - where;
          memmove(buffer.bytes + buffer.end - diff, buffer.bytes + where, diff);
          buffer.start = buffer.start - diff;
          buffer.end = buffer.end - diff;
     } else {
          size_t diff = where - buffer.start;
          memmove(buffer.bytes + buffer.start, buffer.bytes + buffer.end, diff);
          buffer.start = buffer.start + diff;
          buffer.end = buffer.end + diff;
     }

     return buffer;
}

struct GapBuffer
gap_shrink(struct GapBuffer buffer) {
     free(buffer.bytes);
     buffer.start = 0;
     buffer.end = 0;
     buffer.size = 0;

     return buffer;
}

struct GapBuffer
gapwrite(struct GapBuffer buffer, size_t size, FILE *stream) {
     size_t eof = buffer.size - (buffer.end - buffer.start);
     if (buffer.start < eof) {
          size_t diff = eof - buffer.start;

          memmove(buffer.bytes + buffer.start, buffer.bytes + buffer.end, diff);
          buffer.start = buffer.start + diff;
          buffer.end = buffer.end + diff;
     }

     fwrite(buffer.bytes, 1, size, stream);

     return buffer;
}
