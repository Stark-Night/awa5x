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

#ifndef ABYSS_H
#define ABYSS_H

#include <stdint.h>

struct Bubble {
     int32_t value;
     struct Bubble *head;
     struct Bubble *next;
};

struct Abyss {
     struct Bubble *bubbles;
     int size;
     int used;
     struct Bubble *head;
     struct Bubble *free;

     // a hack, but we need some kind of cached buffer
     char *exbuffer;
     size_t exsize;
};

struct Abyss
abyss_expand(struct Abyss abyss);

struct Abyss
abyss_drop(struct Abyss abyss);

struct Abyss
abyss_push(struct Abyss abyss, struct Bubble bubble);

struct Abyss
abyss_pop(struct Abyss abyss);

struct Abyss
abyss_big_pop(struct Abyss abyss);

struct Bubble
abyss_top(struct Abyss abyss);

struct Abyss
abyss_move(struct Abyss abyss, uint8_t steps);

struct Abyss
abyss_join(struct Abyss abyss, uint8_t size);

struct Abyss
abyss_merge(struct Abyss abyss);

struct Abyss
abyss_clone(struct Abyss abyss);

struct Abyss
abyss_sum(struct Abyss abyss);

struct Abyss
abyss_sub(struct Abyss abyss);

struct Abyss
abyss_mul(struct Abyss abyss);

struct Abyss
abyss_div(struct Abyss abyss);

struct Abyss
abyss_visualize(struct Abyss abyss, FILE *stream);

struct Bubble
bubble_wrap(int32_t value);

int
bubble_double(struct Bubble bubble);

int
bubble_count(struct Bubble bubble);

int
bubble_equals(struct Bubble b1, struct Bubble b2);

int
bubble_lessers(struct Bubble b1, struct Bubble b2);

int
bubble_greaters(struct Bubble b1, struct Bubble b2);

int
bubble_zero(struct Bubble bubble);

#endif
