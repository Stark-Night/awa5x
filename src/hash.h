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

#ifndef HASH_H
#define HASH_H

#include <stdint.h>

enum HashItemState {
     HASH_ITEM_INVALID,
     HASH_ITEM_VALID,
};

struct HashItem {
     void *next;
     char *dupkey;
     size_t dupsize;
     uint32_t value;
     enum HashItemState state;
};

#define HASH_BUCKETS 2048

struct Hash {
     struct HashItem *list[HASH_BUCKETS];
};

struct Hash
hash_insert(struct Hash hash, const void *key, size_t keysize, uint32_t value);

struct HashItem
hash_retrieve(struct Hash hash, const void *key, size_t keysize);

struct Hash
hash_close(struct Hash hash);

#endif
