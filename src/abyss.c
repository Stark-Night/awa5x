#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "abyss.h"

struct Abyss
abyss_expand(struct Abyss abyss) {
     if (NULL == abyss.bubbles) {
          abyss.bubbles = malloc(32 * sizeof(struct Bubble));
          abyss.size = 32;
          abyss.top = 0;

          if (NULL == abyss.bubbles) {
               abort();
          }

          return abyss;
     }

     int ns = abyss.size * 2;
     size_t nss = ns * sizeof(struct Bubble);
     if (nss <= abyss.size * sizeof(struct Bubble)) {
          abort();
     }

     abyss.bubbles = realloc(abyss.bubbles, nss);
     abyss.size = ns;

     if (NULL == abyss.bubbles) {
          abort();
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
     abyss.top = 0;

     return abyss;
}

struct Abyss
abyss_push(struct Abyss abyss, struct Bubble bubble) {
     if (abyss.top + 2 >= abyss.size) {
          abyss = abyss_expand(abyss);
     }

     abyss.bubbles[abyss.top] = bubble;
     abyss.top = abyss.top + 1;

     return abyss;
}

struct Abyss
abyss_pop(struct Abyss abyss) {
     if (0 >= abyss.top) {
          abort(); // can only happen if the interpreter has bugs, so
                   // abort is the right move
     }

     abyss.top = abyss.top - 1;

     return abyss;
}

struct Bubble
abyss_top(struct Abyss abyss) {
     if (0 >= abyss.top) {
          abort(); // same as above
     }

     struct Bubble bubble = abyss.bubbles[abyss.top - 1];
     return bubble;
}

struct Bubble
bubble_wrap(int8_t value) {
     struct Bubble bubble = { 0 };
     bubble.value = value;

     return bubble;
}
