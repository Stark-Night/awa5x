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
#include "hash.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     struct Hash hash = { 0 };

     hash = hash_insert(hash, "foobar", 3);
     hash = hash_insert(hash, "uu", 12);
     hash = hash_insert(hash, "very long with spaces and ÜTF-8", 0);

     struct HashItem item = { 0 };

     item = hash_retrieve(hash, "foobar");
     abort_when(HASH_ITEM_INVALID == item.state || 3 != item.value);

     item = hash_retrieve(hash, "uu");
     abort_when(HASH_ITEM_INVALID == item.state || 12 != item.value);

     item = hash_retrieve(hash, "very long with spaces and ÜTF-8");
     abort_when(HASH_ITEM_INVALID == item.state || 0 != item.value);

     hash = hash_close(hash);

     return 0;
}
