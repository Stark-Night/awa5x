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

#ifndef ALINE_H
#define ALINE_H

#include <stdint.h>

#define ALINE_FLAG_BREAK 0x1

struct ALineItem {
     int8_t code;
     uint32_t parameter;
     uint32_t address;
     uint16_t flags;
};

struct ALine {
     struct ALineItem *items;
     size_t size;
     size_t capacity;
};

struct ALine
aline_start(struct ALine aline);

struct ALine
aline_end(struct ALine aline);

struct ALine
aline_track(struct ALine aline, struct ALineItem item);

struct ALine
aline_reset(struct ALine aline);

struct ALine
aline_change_flags_at(struct ALine aline, size_t index, uint16_t flags);

#endif
