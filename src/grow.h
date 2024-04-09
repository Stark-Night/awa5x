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

#ifndef GROW_H
#define GROW_H

#include <stdint.h>

struct GrowBuffer {
     int8_t *bytes;
     size_t size;
     size_t capacity;
};

struct GrowBuffer
append_buffer(struct GrowBuffer buffer, void *data, size_t size);

struct GrowBuffer
reset_buffer(struct GrowBuffer buffer);

struct GrowBuffer
shrink_buffer(struct GrowBuffer buffer);

#endif
