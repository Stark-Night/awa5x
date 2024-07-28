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

#ifndef EXTERN_H
#define EXTERN_H

struct ExternResult {
     enum ExternCode {
          EXTERN_YES,
          EXTERN_NO,
     } code;
     struct Abyss state;
};

struct ExternResult
load_dyn(struct Abyss abyss);

struct ExternResult
call_dyn(struct Abyss abyss);

#endif
