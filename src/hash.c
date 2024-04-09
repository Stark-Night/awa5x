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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

static size_t
hash_key_calc(const char *key) {
     return (strlen(key) % HASH_BUCKETS);
}

struct Hash
hash_insert(struct Hash hash, const char *key, uint32_t value) {
     struct HashItem *item = malloc(sizeof(struct HashItem));
     if (NULL == item) {
          return hash;
     }

     item->value = value;
     item->next = NULL;
     item->dupkey = strdup(key);
     item->state = HASH_ITEM_VALID;

     size_t lkey = hash_key_calc(key);
     if (NULL != hash.list[lkey]) {
          item->next = hash.list[lkey];
     }

     hash.list[lkey] = item;
     return hash;
}

struct HashItem
hash_retrieve(struct Hash hash, const char *key) {
     struct HashItem dup = { 0 };

     size_t lkey = hash_key_calc(key);
     if (NULL == hash.list[lkey]) {
          return dup;
     }

     for (struct HashItem *item=hash.list[lkey]; NULL!=item; item=item->next) {
          if (0 != strcmp(key, item->dupkey)) {
               continue;
          }

          dup.value = item->value;
          dup.state = HASH_ITEM_VALID;
          break;
     }

     return dup;
}

struct Hash
hash_close(struct Hash hash) {
     for (size_t i=0; i<HASH_BUCKETS; ++i) {
          struct HashItem *item = hash.list[i];
          while (NULL != item) {
               struct HashItem *next = item->next;
               free(item);
               item = next;
          }
     }

     memset(hash.list, 0, HASH_BUCKETS);

     return hash;
}
