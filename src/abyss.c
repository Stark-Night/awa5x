#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "abyss.h"

struct Abyss
abyss_expand(struct Abyss abyss) {
     if (NULL == abyss.bubbles) {
          abyss.bubbles = malloc(32 * sizeof(struct Bubble));
          abyss.size = 32;
          abyss.used = 0;

          if (NULL == abyss.bubbles) {
               abort();
          }

          abyss.free = abyss.bubbles;

          struct Bubble *prev = abyss.free;
          struct Bubble *bubble = abyss.bubbles + 1;
          for (int i=1; i<32; ++i) {
               prev->next = bubble;
               prev = bubble;
               bubble = abyss.bubbles + i;
          }

          return abyss;
     }

     int ps = abyss.size;
     int ns = ps * 2;
     size_t nss = ns * sizeof(struct Bubble);
     if (nss <= abyss.size * sizeof(struct Bubble)) {
          abort();
     }

     abyss.bubbles = realloc(abyss.bubbles, nss);
     abyss.size = ns;

     if (NULL == abyss.bubbles) {
          abort();
     }

     for (int i=ps; i<abyss.size; ++i) {
          struct Bubble *b = abyss.bubbles + i;
          b->next = abyss.free;
          abyss.free = b;
     }

     return abyss;
}

struct Abyss
abyss_drop(struct Abyss abyss) {
     if (NULL != abyss.bubbles) {
          free(abyss.bubbles);
     }

     abyss.bubbles = NULL;
     abyss.size = 0;
     abyss.used = 0;
     abyss.head = NULL;
     abyss.free = NULL;

     return abyss;
}

struct Abyss
abyss_push(struct Abyss abyss, struct Bubble bubble) {
     if (abyss.used + 2 >= abyss.size) {
          abyss = abyss_expand(abyss);
     }

     struct Bubble *b = abyss.free;
     abyss.free = b->next;

     b->value = bubble.value;
     b->next = abyss.head;
     abyss.head = b;

     abyss.used = abyss.used + 1;

     return abyss;
}

struct Abyss
abyss_pop(struct Abyss abyss) {
     if (0 >= abyss.used) {
          abort(); // can only happen if the interpreter has bugs, so
                   // abort is the right move
     }

     struct Bubble *bubble = abyss.head;
     abyss.head = bubble->next;
     bubble->next = abyss.free;
     abyss.free = bubble;

     abyss.used = abyss.used - 1;

     return abyss;
}

struct Bubble
abyss_top(struct Abyss abyss) {
     if (0 >= abyss.used) {
          abort(); // same as above
     }

     struct Bubble bubble = { 0 };
     struct Bubble *top = abyss.head;

     bubble.value = top->value;
     return bubble;
}

struct Abyss
abyss_move(struct Abyss abyss, uint8_t steps) {
     // the uint8_t type allows a step of 255 when using a parameter
     // from the interpeter
     if (0 == steps) {
          return abyss;
     }

     struct Bubble *bubble = abyss.head;
     if (NULL == bubble || NULL == bubble->next) {
          return abyss;
     }

     abyss.head = bubble->next;

     struct Bubble *prev = NULL;
     struct Bubble *next = bubble->next;
     for (int i=steps; i>=0; --i) {
          if (NULL == next || 0 >= i) {
               prev->next = bubble;
               break;
          }

          prev = next;
          next = next->next;
     }

     return abyss;
}

struct Abyss
abyss_join(struct Abyss abyss, uint8_t size) {
     // the uint8_t type allows a step of 255 when using a parameter
     // from the interpeter
     if (0 == size) {
          return abyss;
     }

     struct Bubble *bubble = abyss.free;
     abyss.free = abyss.free->next;
     abyss.used = abyss.used + 1;

     bubble->head = abyss.head;
     abyss.head = abyss.head->next;

     for (int i=0; i<size && NULL!=abyss.head; ++i) {
          abyss.head = abyss.head->next;
     }

     bubble->next = abyss.head;
     abyss.head = bubble;

     return abyss;
}

struct Bubble
bubble_wrap(int8_t value) {
     struct Bubble bubble = { 0 };
     bubble.value = value;

     return bubble;
}