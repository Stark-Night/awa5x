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

#ifndef UTF8_H
#define UTF8_H

#include <stdint.h>

struct UTF8Result {
     uint32_t point;
     int bytes;
};

struct UTF8Result
utf8_decode(const void *buffer);

// A or a
#define IS_A(decoded) ((0x61 == ((decoded).point)) || (0x41 == ((decoded).point)))

// W of w
#define IS_W(decoded) ((0x77 == ((decoded).point)) || (0x57 == ((decoded).point)))

// space or newline
#define IS_S(decoded) ((0x20 == ((decoded).point)) || (0x0A == ((decoded).point)))

// ~
#define IS_T(decoded) ((0x7E == ((decoded).point)))

// ;
#define IS_C(decoded) ((0x3B == ((decoded).point)))

// newline
#define IS_N(decoded) ((0x0A == ((decoded).point)))

#endif
