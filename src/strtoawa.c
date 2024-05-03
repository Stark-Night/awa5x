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

#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

#include "strtoawa.h"

int32_t
strtoawa(const char *restrict nptr, char **restrict endptr) {
     const char *p = nptr;
     int32_t n = 0;
     int negative = 0;

     while (0x00 != *p && isspace(*p)) {
          p = p + 1;
     }

     if (0x00 == *p) {
          if (NULL != endptr) {
               *endptr = (char *)nptr;
          }

          return n;
     }

     if (0x7E == *p) {
          negative = 1;
          p = p + 1;
     }

     int run = 1;
     while (0x00 != *p && 0 != run) {
          int value = 0;

          switch (*p) {
          case 0x30: value = 0; break;
          case 0x31: value = 1; break;
          case 0x32: value = 2; break;
          case 0x33: value = 3; break;
          case 0x34: value = 4; break;
          case 0x35: value = 5; break;
          case 0x36: value = 6; break;
          case 0x37: value = 7; break;
          case 0x38: value = 8; break;
          case 0x39: value = 9; break;
          default:
               run = 0;
               continue;
          }

          uint32_t newn = ((uint32_t)n) * 10 + value;

          if (newn > INT32_MAX) {
               n = (0 == negative) ? INT32_MAX : INT32_MIN;
               run = 0;
               continue;
          }

          n = newn;
          p = p + 1;
     }

     if (NULL != endptr) {
          *endptr = (char *)p;
     }

     return (0 == negative) ? n : -n;
}
