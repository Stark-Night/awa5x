#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "abyss.h"

struct Page {
     struct Bubble *bubble;
     struct Abyss state;
};

static struct Page
take_free_bubble(struct Abyss abyss) {
     struct Page page = { 0 };
     page.state = abyss;

     if (page.state.used + 2 >= page.state.size) {
          page.state = abyss_expand(page.state);
     }

     page.bubble = page.state.free;

     page.state.free = page.bubble->next;
     page.state.used = page.state.used + 1;

     page.bubble->head = NULL;
     page.bubble->next = NULL;

     return page;
}

static struct Page
give_free_bubble(struct Abyss abyss, struct Bubble *bubble) {
     struct Page page = { 0 };
     page.bubble = bubble;
     page.state = abyss;

     page.bubble->next = page.state.free;
     page.state.free = page.bubble;
     page.state.used = page.state.used - 1;

     return page;
}

static struct Page
clone_bubble(struct Abyss abyss, struct Bubble *bubble) {
     struct Page page = take_free_bubble(abyss);
     struct Bubble *clone = page.bubble;

     if (0 == bubble_double(*bubble)) {
          clone->value = bubble->value;
          return page;
     }

     page = clone_bubble(page.state, bubble->head);
     clone->head = page.bubble;

     struct Bubble *cursor = clone->head;
     for (struct Bubble *b=bubble->head->next; NULL!=b; b=b->next) {
          page = clone_bubble(page.state, b);

          cursor->next = page.bubble;
          cursor = cursor->next;
     }

     page.bubble = clone;
     return page;
}

static struct Abyss
open_abyss(struct Abyss abyss, int size) {
     size_t bsize = size * sizeof(struct Bubble);
     if (bsize <= abyss.size * sizeof(struct Bubble)) {
          abort();
     }

     abyss.bubbles = malloc(bsize);
     abyss.size = size;
     abyss.used = 0;
     abyss.head = NULL;

     if (NULL == abyss.bubbles) {
          abort();
     }

     return abyss;
}

static struct Abyss
generate_free_chain(struct Abyss abyss) {
     abyss.free = abyss.bubbles;

     struct Bubble *prev = abyss.free;
     struct Bubble *bubble = abyss.bubbles + 1;
     for (int i=1; i<abyss.size; ++i) {
          prev->next = bubble;
          prev = bubble;
          bubble = abyss.bubbles + i;
     }

     return abyss;
}

struct Abyss
abyss_expand(struct Abyss abyss) {
     if (NULL == abyss.bubbles) {
          abyss = open_abyss(abyss, 32);
          abyss = generate_free_chain(abyss);

          return abyss;
     }

     struct Bubble *cloned = abyss.bubbles;
     struct Bubble *chead = abyss.head;
     struct Bubble *head = NULL;

     abyss = open_abyss(abyss, abyss.size * 2);
     abyss = generate_free_chain(abyss);

     while (NULL != chead) {
          struct Page page = clone_bubble(abyss, chead);
          struct Bubble *b = page.bubble;

          abyss = page.state;

          if (NULL == head) {
               head = b;
               head->next = NULL;
               abyss.head = head;
          } else {
               head->next = b;
          }

          head = b;
          chead = chead->next;
     }

     free(cloned);

     return abyss;
}

struct Abyss
abyss_drop(struct Abyss abyss) {
     if (NULL != abyss.bubbles) {
          free(abyss.bubbles);
     }
     if (NULL != abyss.exbuffer) {
          free(abyss.exbuffer);
          abyss.exbuffer = NULL;
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
     struct Page page = take_free_bubble(abyss);

     struct Bubble *b = page.bubble;
     b->value = bubble.value;
     b->next = abyss.head;
     page.state.head = b;

     return page.state;
}

struct Abyss
abyss_pop(struct Abyss abyss) {
     if (0 >= abyss.used) {
          abort(); // can only happen if the interpreter has bugs, so
                   // abort is the right move
     }

     struct Bubble *bubble = abyss.head;
     abyss.head = bubble->next;
     struct Page page = give_free_bubble(abyss, bubble);

     if (NULL != page.bubble->head) {
          struct Bubble *head = page.bubble->head;
          struct Bubble *cursor = head;
          while (NULL != cursor->next) {
               cursor = cursor->next;
          }

          cursor->next = page.state.head;
          page.state.head = head;
     }

     page.bubble->head = NULL;

     return page.state;
}

struct Bubble
abyss_top(struct Abyss abyss) {
     if (0 >= abyss.used) {
          abort(); // same as above
     }

     struct Bubble *top = abyss.head;
     struct Bubble bubble = *top;

     return bubble;
}

struct Abyss
abyss_move(struct Abyss abyss, uint8_t steps) {
     if (0 >= abyss.used) {
          abort(); // signal the user; recovering doesn't seem like a good idea
     }

     // the uint8_t type allows a step of 255 when using a parameter
     // from the interpeter
     struct Bubble *bubble = abyss.head;
     if (NULL == bubble->next) {
          return abyss;
     }

     abyss.head = bubble->next;

     if (0 == steps) {
          struct Bubble *cursor = abyss.head;

          while (NULL != cursor->next) {
               cursor = cursor->next;
          }

          cursor->next = bubble;
          return abyss;
     }

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
     if (0 >= abyss.used) {
          abort(); // same as above
     }

     // the uint8_t type allows a step of 255 when using a parameter
     // from the interpeter

     if (0 == size) {
          return abyss;
     }

     struct Page page = take_free_bubble(abyss);

     page.bubble->head = page.state.head;
     page.state.head = page.state.head->next;

     struct Bubble *cursor = page.state.head;
     for (int i=0; i<size && NULL!=page.state.head; ++i) {
          cursor = page.state.head;
          page.state.head = page.state.head->next;
     }
     cursor->next = NULL;

     page.bubble->next = page.state.head;
     page.state.head = page.bubble;

     return page.state;
}

struct Abyss
abyss_merge(struct Abyss abyss) {
     if (0 >= abyss.used) {
          abort(); // same as above
     }

     if (NULL == abyss.head->next) {
          return abyss;
     }

     struct Bubble *b1 = abyss.head;
     abyss.head = b1->next;
     struct Bubble *b2 = abyss.head;
     abyss.head = b2->next;

     struct Page page = { 0 };
     page.state = abyss;

     if (0 == bubble_double(*b1) && 0 == bubble_double(*b2)) {
          page = take_free_bubble(abyss);

          page.bubble->head = b1;
          page.bubble->head->next = b2;
          b2->next = NULL;

          page.bubble->next = page.state.head;
          page.state.head = page.bubble;
     } else if (0 == bubble_double(*b1)) {
          page = take_free_bubble(abyss);

          page.bubble->head = b1;
          b1->next = b2->head;
          page.bubble->next = page.state.head;
          page.state.head = page.bubble;

          page = give_free_bubble(page.state, b2);
     } else if (0 == bubble_double(*b2)) {
          struct Bubble *cursor = b1->head;
          while (NULL != cursor->next) {
               cursor = cursor->next;
          }
          cursor->next = b2;
          b1->next = page.state.head;
          page.state.head = b1;

          page = give_free_bubble(page.state, b2);
     } else {
          struct Bubble *cursor = b1->head;
          while (NULL != cursor->next) {
               cursor = cursor->next;
          }
          cursor->next = b2->head;

          b1->next = page.state.head;
          page.state.head = b1;

          page = give_free_bubble(page.state, b2);
     }

     return page.state;
}

struct Abyss
abyss_clone(struct Abyss abyss) {
     if (0 >= abyss.used) {
          abort(); // same as above
     }

     if (NULL == abyss.head) {
          return abyss;
     }

     struct Page page = take_free_bubble(abyss);
     struct Bubble *bubble = abyss.head;
     struct Bubble *clone = page.bubble;

     clone->next = bubble;
     page.state.head = clone;

     if (0 == bubble_double(*bubble)) {
          clone->value = bubble->value;
          return page.state;
     }

     page = clone_bubble(page.state, bubble->head);
     clone->head = page.bubble;

     struct Bubble *cursor = clone->head;
     for (struct Bubble *b=bubble->head->next; NULL!=b; b=b->next) {
          page = clone_bubble(page.state, b);

          cursor->next = page.bubble;
          cursor = cursor->next;
     }

     return page.state;
}

struct Bubble
bubble_wrap(int8_t value) {
     struct Bubble bubble = { 0 };
     bubble.value = value;

     return bubble;
}

int
bubble_double(struct Bubble bubble) {
     return (NULL != bubble.head);
}

int
bubble_count(struct Bubble bubble) {
     if (0 == bubble_double(bubble)) {
          return 0;
     }

     int counter = 0;
     for (struct Bubble *b=bubble.head; NULL!=b; b=b->next) {
          counter = counter + 1;
     }

     return counter;
}
