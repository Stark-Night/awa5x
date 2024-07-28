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
#include <stdint.h>
#include <string.h>
#include "opcodes.h"
#include "abyss.h"
#include "extern.h"

#if defined(_WIN64) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

struct Entry {
     char *name;
     int loaded;
#if defined(_WIN64) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
     HMODULE handle;
#else
     void *handle;
#endif
     struct Entry *next;
};

struct Alloc {
     int used;
     void *buffer;
     uint32_t size;
};

struct AllocCache {
     struct Alloc *allocs;
     uint32_t capacity;
};

struct BubbleVector {
     struct Alloc *alloc;
     uint32_t count;
};

typedef int
(external_fn_t)(int64_t *in,
                uint32_t insize,
                int64_t **out,
                uint32_t *outsize);

#define CACHE_TABLE_SIZE 16384
struct Entry *module_table[CACHE_TABLE_SIZE] = { 0 };
struct AllocCache alloc_cache = { 0 };

static uint32_t
next_power(uint32_t n) {
     uint32_t v = n;
     v = v - 1;
     v = v | v >> 1;
     v = v | v >> 2;
     v = v | v >> 4;
     v = v | v >> 8;
     v = v | v >> 16;
     v = v + 1;
     return v;
}

static int
grow_alloc_cache(struct AllocCache *cache, uint32_t expected) {
     if (NULL == cache->allocs) {
          cache->allocs = malloc(1024 * sizeof(struct Alloc));
          if (NULL == cache->allocs) {
               // out of memory, let's just crash...
               abort();
          }

          cache->capacity = 1024;
     }

     if (expected < cache->capacity) {
          return 0;
     }

     uint32_t newcap = cache->capacity;
     uint32_t newsize = cache->capacity * sizeof(struct Alloc);

     while (cache->capacity < expected) {
          newcap = cache->capacity * 2;
          newsize = newcap * sizeof(struct Alloc);

          if (newcap <= cache->capacity
              || newsize <= cache->capacity * sizeof(struct Alloc)) {
               // out of memory, let's just crash...
               abort();
          }
     }

     cache->allocs = realloc(cache->allocs, newsize);
     if (NULL == cache->allocs) {
          // out of memory, let's just crash...
          abort();
     }

     cache->capacity = newcap;

     return 0;
}

static struct Alloc *
find_alloc_for_size(struct AllocCache *cache, uint32_t expected) {
     struct Alloc *retval = NULL;
     uint32_t cursor = expected;

     while (NULL == retval) {
          if (0 != grow_alloc_cache(cache, cursor)) {
               return NULL;
          }

          struct Alloc *alloc = cache->allocs + cursor;
          if (0 != alloc->used) {
               cursor = next_power(cursor + 1);
               if (cursor <= expected) {
                    // we somehow filled the whole cache after the
                    // expected size; let's abort and be loud about it
                    abort();
               }

               continue;
          }

          alloc->size = cursor;
          alloc->used = 1;

          retval = alloc;
     }

     if (NULL == retval->buffer) {
          retval->buffer = malloc(retval->size);
     }

     return retval;
}

static struct Alloc *
recycle_alloc(struct AllocCache *cache, struct Alloc *alloc) {
     if (0 == alloc->used) {
          return alloc;
     }

     alloc->used = 0;

     return alloc;
}

static struct Alloc *
make_bubble_string(struct Bubble *bubble) {
     int size = bubble_count(*bubble);

     // find the next power of two to get a slightly better memory
     // layout
     uint32_t finalsize = next_power((0 == size) ? 2 : size + 1);
     struct Alloc *alloc = find_alloc_for_size(&alloc_cache, finalsize);

     char *buffer = alloc->buffer;
     if (0 == size) {
          if (bubble->value >= NALNUM) {
               // technically wrong, but further usages will result in
               // failures unless someone decides to be extra clever
               // (read: that person intentionally wants to be
               // disruptive and malicious).
               buffer[0] = 0x3F;
          } else {
               buffer[0] = ALPHABET[bubble->value];
          }

          buffer[1] = 0x00;
     } else {
          uint32_t i = 0;
          for (struct Bubble *b=bubble->head; NULL!=b; b=b->next) {
               if (0 != bubble_double(*b)) {
                    // ignore, but warn the user
                    fprintf(stderr, "double bubble when single expected; ignoring\n");
                    continue;
               }

               if (b->value >= NALNUM) {
                    // see above
                    buffer[i] = 0x3F;
               } else {
                    buffer[i] = ALPHABET[b->value];
               }

               i = i + 1;
          }

          buffer[i] = 0x00;
     }

     return alloc;
}

static uint32_t
count_bubble_vector(struct Bubble *bubble) {
     if (0 == bubble_double(*bubble)) {
          return 1;
     }

     uint32_t count = 2;
     for (struct Bubble *b=bubble->head; NULL!=b; b=b->next) {
          count = count + count_bubble_vector(b);
     }

     return count;
}

static int64_t *
flatten_bubble(struct Bubble *bubble, int64_t *buffer) {
     if (0 == bubble_double(*bubble)) {
          buffer[0] = bubble->value;
          return buffer + 1;
     }

     buffer[0] = INT64_MIN;
     for (struct Bubble *b=bubble->head; NULL!=b; b=b->next) {
          buffer = flatten_bubble(b, buffer);
     }
     buffer[0] = INT64_MAX;

     return buffer + 1;
}

static int
fill_bubble_vector(struct Bubble *bubble, struct Alloc *alloc) {
     int64_t *buffer = alloc->buffer;

     if (0 == bubble_double(*bubble)) {
          buffer[0] = bubble->value;
          return 0;
     }

     for (struct Bubble *b=bubble->head; NULL!=b; b=b->next) {
          buffer = flatten_bubble(b, buffer);
     }

     return 0;
}

struct BubbleVector
make_bubble_vector(struct Bubble *bubble) {
     uint32_t count = count_bubble_vector(bubble);

     // find the next power of two to get a slightly better memory
     // layout
     uint32_t finalsize = next_power(count * sizeof(int64_t));
     struct Alloc *alloc = find_alloc_for_size(&alloc_cache, finalsize);
     if (0 != fill_bubble_vector(bubble, alloc)) {
          // unsure how to handle it to be honest
          abort();
     }

     struct BubbleVector retval = { 0 };
     retval.alloc = alloc;
     retval.count = count;
     return retval;
}

static struct Entry *
find_entry(char *name) {
     // djb2
     uint32_t hash = 0x1505;
     for (char *p=name; 0x00!=*p; p=p+1) {
          int c = *p;
          hash = ((hash << 0x05) + hash) + c;
     }

     uint32_t key = hash % CACHE_TABLE_SIZE;
     struct Entry *existing = module_table[key];
     if (NULL != existing) {
          struct Entry *cursor = existing;
          while (NULL != cursor) {
               if (0 == strcmp(name, cursor->name)) {
                    return cursor;
               }

               existing = cursor;
               cursor = cursor->next;
          }
     }

     struct Entry *new = malloc(sizeof(struct Entry));
     if (NULL == new) {
          // out of memory, let's just crash...
          abort();
     }

     new->name = strdup(name);
     new->loaded = 0;
     if (NULL != existing) {
          existing->next = new;
     } else {
          module_table[key] = new;
     }

     return new;
}

static int
load_entry(struct Entry *entry) {
     if (0 != entry->loaded) {
          return 0;
     }

#if defined(_WIN64) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
     entry->handle = LoadLibrary(entry->name);
#else
     entry->handle = dlopen(entry->name, RTLD_LAZY);
#endif

     if (NULL == entry->handle) {
          fprintf(stderr, "cannot load %s\n", entry->name);
          return 1;
     }

     entry->loaded = 1;

     return 0;
}

struct ExternResult
load_dyn(struct Abyss abyss) {
     struct ExternResult result = { 0 };
     result.state = abyss;

     struct Bubble top = abyss_top(result.state);
     struct Alloc *alloc = make_bubble_string(&top);
     char *name = alloc->buffer;
     if (NULL == name) {
          result.code = EXTERN_NO;
          return result;
     }

     struct Entry *entry = find_entry(name);
     int r = load_entry(entry);
     if (0 != r) {
          result.code = EXTERN_NO;
          return result;
     }

     recycle_alloc(&alloc_cache, alloc);

     result.code = EXTERN_YES;
     result.state = abyss_big_pop(result.state);
     return result;
}

struct ExternResult
call_dyn(struct Abyss abyss) {
     struct ExternResult result = { 0 };
     result.state = abyss;

     struct Bubble top = abyss_top(result.state);
     if (0 == bubble_double(top)) {
          result.code = EXTERN_NO;
          return result;
     }

     struct Bubble *lib = top.head;
     struct Bubble *fn = lib->next;
     struct Bubble *args = fn->next;

     struct Alloc *libname = make_bubble_string(lib);
     struct Alloc *callname = make_bubble_string(fn);
     struct BubbleVector argvec = { 0 };

     if (NULL != args) {
          argvec = make_bubble_vector(args);
     }

     struct Entry *entry = find_entry(libname->buffer);
     fprintf(stderr, "%s %s\n",
             (char *)(libname->buffer),
             (char *)(callname->buffer));
     if (0 == entry->loaded) {
          // programming error somewhere, die to be loud about it
          abort();
     }

     external_fn_t *external_fn = NULL;
#if defined(_WIN64) || defined(_WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
     external_fn = GetProcAddress(entry->handle, callname);
#else
     external_fn = dlsym(entry->handle, callname->buffer);
#endif

     if (NULL == external_fn) {
          result.code = EXTERN_NO;
          return result;
     }

     int64_t *inbuf = (NULL != argvec.alloc) ?
          argvec.alloc->buffer :
          NULL;

     int64_t *retbuf = NULL;
     uint32_t retsize = 0;
     int r = (*external_fn)(inbuf,
                            argvec.count,
                            &retbuf,
                            &retsize);
     if (0 != r) {
          result.code = EXTERN_NO;
          return result;
     }

     recycle_alloc(&alloc_cache, libname);
     recycle_alloc(&alloc_cache, callname);
     if (NULL != argvec.alloc) {
          recycle_alloc(&alloc_cache, argvec.alloc);
     }

     result.code = EXTERN_YES;
     result.state = abyss_big_pop(result.state);

     if (0 != retsize && NULL != retbuf) {
          // push result into abyss somehow
     }

     return result;
}
