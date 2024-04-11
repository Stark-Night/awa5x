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

#ifndef GAP_H
#define GAP_H

#include <stdint.h>

struct GapBuffer {
     int8_t *bytes;
     size_t size;
     size_t start;
     size_t end;
};

struct GapBuffer
gap_append(struct GapBuffer buffer, void *data, size_t size);

struct GapBuffer
gap_move(struct GapBuffer buffer, size_t where);

struct GapBuffer
gap_shrink(struct GapBuffer buffer);

#endif
