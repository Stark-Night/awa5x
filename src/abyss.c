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
#include "abyss.h"

typedef int32_t (*bubble_plain_op)(
     struct Bubble *b1, struct Bubble *b2);

typedef int (*bubble_comparer)(
     struct Bubble b1, struct Bubble b2);

struct Page {
     struct Bubble *bubble;
     struct Abyss state;
};

struct Inflatable {
     struct Page page;
     uint32_t examined;
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

     page.bubble->value = 0;
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
give_double_bubble(struct Abyss abyss, struct Bubble *bubble) {
     struct Page page = { 0 };
     page.state = abyss;

     if (0 == bubble_double(*bubble)) {
          page = give_free_bubble(page.state, bubble);
          return page;
     }

     struct Bubble *cursor = bubble->head;
     while (NULL != cursor) {
          struct Bubble *next = cursor->next;
          page = give_double_bubble(page.state, cursor);
          cursor = next;
     }

     page = give_free_bubble(page.state, bubble);
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
          abyss = open_abyss(abyss, 256);
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
     b->next = page.state.head;
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

struct Abyss
abyss_big_pop(struct Abyss abyss) {
     struct Bubble *b = abyss.head;
     abyss.head = b->next;

     struct Page page = give_double_bubble(abyss, b);

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

     // detach bubble to move from abyss
     abyss.head = bubble->next;
     bubble->next = NULL;

     if (0 == steps) {
          // bring bubble to bottom of abyss

          struct Bubble *cursor = abyss.head;
          while (NULL != cursor->next) {
               cursor = cursor->next;
          }

          cursor->next = bubble;
          return abyss;
     }

     // move bubble steps-1 positions
     struct Bubble *cursor = abyss.head;
     for (uint8_t i=(steps-1); i>0 && NULL!=cursor->next; --i) {
          cursor = cursor->next;
     }
     bubble->next = cursor->next;
     cursor->next = bubble;

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

     struct Bubble *cursor = page.bubble->head;
     for (int i=1; i<size && NULL!=cursor; ++i) {
          cursor->next = page.state.head;
          cursor = cursor->next;

          if (NULL != page.state.head) {
               page.state.head = page.state.head->next;
          }
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

static struct Page
apply_plain_op(struct Abyss abyss, struct Bubble *b1, struct Bubble *b2, bubble_plain_op op) {
     struct Page page = { 0 };
     page.state = abyss;

     if (0 == bubble_double(*b1) && 0 == bubble_double(*b2)) {
          page = take_free_bubble(page.state);
          page.bubble->value = op(b1, b2);

          return page;
     } else if (0 == bubble_double(*b2)) {
          page = take_free_bubble(page.state);

          struct Bubble *head = page.bubble;
          struct Bubble *cursor = NULL;

          for (struct Bubble *b=b1->head; NULL!=b; b=b->next) {
               page = apply_plain_op(page.state, b, b2, op);

               if (NULL == cursor) {
                    head->head = page.bubble;
                    cursor = head->head;
               } else {
                    cursor->next = page.bubble;
                    cursor = cursor->next;
               }
          }

          page.bubble = head;
          return page;
     } else if (0 == bubble_double(*b1)) {
          page = take_free_bubble(page.state);

          struct Bubble *head = page.bubble;
          struct Bubble *cursor = NULL;

          for (struct Bubble *b=b2->head; NULL!=b; b=b->next) {
               page = apply_plain_op(page.state, b1, b, op);

               if (NULL == cursor) {
                    head->head = page.bubble;
                    cursor = head->head;
               } else {
                    cursor->next = page.bubble;
                    cursor = cursor->next;
               }
          }

          page.bubble = head;
          return page;
     } else {
          page = take_free_bubble(page.state);

          struct Bubble *head = page.bubble;
          struct Bubble *cursor = NULL;

          struct Bubble *b = b1->head;
          struct Bubble *d = b2->head;
          while (NULL != b && NULL != d) {
               page = apply_plain_op(page.state, b, d, op);

               if (NULL == cursor) {
                    head->head = page.bubble;
                    cursor = head->head;
               } else {
                    cursor->next = page.bubble;
                    cursor = cursor->next;
               }

               b = b->next;
               d = d->next;
          }

          page.bubble = head;
          return page;
     }

     return page;
}

static int32_t
bubble_plain_sum(struct Bubble *b1, struct Bubble *b2) {
     return b1->value + b2->value;
}

static int32_t
bubble_plain_diff(struct Bubble *b1, struct Bubble *b2) {
     return b1->value - b2->value;
}

static int32_t
bubble_plain_mul(struct Bubble *b1, struct Bubble *b2) {
     return b1->value * b2->value;
}

struct Abyss
abyss_sum(struct Abyss abyss) {
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

     struct Page page = apply_plain_op(abyss, b1, b2, &bubble_plain_sum);
     page.bubble->next = page.state.head;
     page.state.head = page.bubble;

     page = give_free_bubble(page.state, b1);
     page = give_free_bubble(page.state, b2);

     return page.state;
}

struct Abyss
abyss_sub(struct Abyss abyss) {
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

     struct Page page = apply_plain_op(abyss, b1, b2, &bubble_plain_diff);
     page.bubble->next = page.state.head;
     page.state.head = page.bubble;

     page = give_free_bubble(page.state, b1);
     page = give_free_bubble(page.state, b2);

     return page.state;
}

struct Abyss
abyss_mul(struct Abyss abyss) {
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

     struct Page page = apply_plain_op(abyss, b1, b2, &bubble_plain_mul);
     page.bubble->next = page.state.head;
     page.state.head = page.bubble;

     page = give_free_bubble(page.state, b1);
     page = give_free_bubble(page.state, b2);

     return page.state;
}

static struct Page
apply_div_op(struct Abyss abyss, struct Bubble *b1, struct Bubble *b2) {
     struct Page page = { 0 };
     page.state = abyss;

     if (0 == bubble_double(*b1) && 0 == bubble_double(*b2)) {
          struct Bubble *head = NULL;
          struct Bubble *reminder = NULL;
          struct Bubble *quotient = NULL;

          page = take_free_bubble(page.state);
          head = page.bubble;

          page = take_free_bubble(page.state);
          reminder = page.bubble;

          page = take_free_bubble(page.state);
          quotient = page.bubble;

          ldiv_t qr = ldiv(b1->value, b2->value);
          reminder->value = (int32_t)qr.rem;
          quotient->value = (int32_t)qr.quot;

          quotient->next = reminder;
          head->head = quotient;

          page.bubble = head;
          return page;
     } else if (0 == bubble_double(*b2)) {
          page = take_free_bubble(page.state);

          struct Bubble *head = page.bubble;
          struct Bubble *cursor = NULL;

          for (struct Bubble *b=b1->head; NULL!=b; b=b->next) {
               page = apply_div_op(page.state, b, b2);

               if (NULL == cursor) {
                    head->head = page.bubble;
                    cursor = head->head;
               } else {
                    cursor->next = page.bubble;
                    cursor = cursor->next;
               }
          }

          page.bubble = head;
          return page;
     } else if (0 == bubble_double(*b1)) {
          page = take_free_bubble(page.state);

          struct Bubble *head = page.bubble;
          struct Bubble *cursor = NULL;

          for (struct Bubble *b=b2->head; NULL!=b; b=b->next) {
               page = apply_div_op(page.state, b1, b);

               if (NULL == cursor) {
                    head->head = page.bubble;
                    cursor = head->head;
               } else {
                    cursor->next = page.bubble;
                    cursor = cursor->next;
               }
          }

          page.bubble = head;
          return page;
     } else {
          page = take_free_bubble(page.state);

          struct Bubble *head = page.bubble;
          struct Bubble *cursor = NULL;

          struct Bubble *b = b1->head;
          struct Bubble *d = b2->head;
          while (NULL != b && NULL != d) {
               page = apply_div_op(page.state, b, d);

               if (NULL == cursor) {
                    head->head = page.bubble;
                    cursor = head->head;
               } else {
                    cursor->next = page.bubble;
                    cursor = cursor->next;
               }

               b = b->next;
               d = d->next;
          }

          page.bubble = head;
          return page;
     }

     return page;
}

struct Abyss
abyss_div(struct Abyss abyss) {
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

     struct Page page = apply_div_op(abyss, b1, b2);
     page.bubble->next = page.state.head;
     page.state.head = page.bubble;

     page = give_free_bubble(page.state, b1);
     page = give_free_bubble(page.state, b2);

     return page.state;
}

static int
bubble_visualize(struct Bubble *bubble, FILE *stream) {
     if (0 == bubble_double(*bubble)) {
          return fprintf(stream, "(0x%x)\n", bubble->value);
     }

     int res = 0;
     res = res + fprintf(stream, "[\n");
     for (struct Bubble *b=bubble->head; NULL!=b; b=b->next) {
          res = res + bubble_visualize(b, stream);
     }
     res = res + fprintf(stream, "]\n");

     return res;
}

struct Abyss
abyss_visualize(struct Abyss abyss, FILE *stream) {
     fprintf(stream, "{\n");

     for (struct Bubble *b=abyss.head; NULL!=b; b=b->next) {
          bubble_visualize(b, stream);
     }

     fprintf(stream, "}\n");

     return abyss;
}

static struct Inflatable
inflate_bubble(struct Abyss abyss, int64_t *buffer, uint32_t size) {
     struct Inflatable page = { 0 };
     page.page.state = abyss;

     page.page = take_free_bubble(page.page.state);
     struct Bubble *top = page.page.bubble;
     struct Bubble *cursor = NULL;

     uint32_t index = 1;
     while (index < size && INT64_MAX != buffer[index]) {
          if (INT64_MIN != buffer[index]) {
               if (INT32_MIN > buffer[index] || INT32_MAX < buffer[index]) {
                    fprintf(stderr, "external value too large\n");
               } else {
                    if (NULL == top->head) {
                         page.page = take_free_bubble(page.page.state);
                         top->head = page.page.bubble;
                         top->head->value = (int32_t)(buffer[index]);

                         cursor = top->head;
                    } else {
                         page.page = take_free_bubble(page.page.state);
                         cursor->next = page.page.bubble;
                         cursor = cursor->next;

                         cursor->value = (int32_t)(buffer[index]);
                    }
               }

               index = index + 1;

               continue;
          }

          page = inflate_bubble(page.page.state,
                                buffer + index,
                                size - index);

          if (NULL == top->head) {
               top->head = page.page.bubble;

               cursor = top->head;
          } else {
               cursor->next = page.page.bubble;
               cursor = cursor->next;
          }

          index = index + page.examined + 1;
     }

     page.page.bubble = top;
     page.examined = index;

     return page;
}

struct Abyss
abyss_external(struct Abyss abyss, int64_t *list, uint32_t size) {
     struct Page page = { 0 };
     page.state = abyss;

     if (0 == size) {
          return page.state;
     }

     if (1 == size) {
          if (INT32_MIN > list[0] || INT32_MAX < list[0]) {
               fprintf(stderr, "external value too large\n");
          } else {
               struct Bubble b = bubble_wrap((int32_t)(list[0]));
               return abyss_push(page.state, b);
          }

          return page.state;
     }

     if (INT64_MIN != list[0] || INT64_MAX != list[size-1]) {
          fprintf(stderr, "improper external value\n");
          return page.state;
     }

     struct Inflatable expand = { 0 };
     expand.page = page;

     expand = inflate_bubble(expand.page.state, list, size);
     struct Bubble *top = expand.page.bubble;

     top->next = expand.page.state.head;
     expand.page.state.head = top;

     return expand.page.state;
}

struct Bubble
bubble_wrap(int32_t value) {
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

static int
bubble_comparator(struct Bubble b1, struct Bubble b2, bubble_comparer f) {
     if (0 == bubble_double(b1) && 0 == bubble_double(b2)) {
          return f(b1, b2);
     } else if (0 != bubble_double(b1) && 0 != bubble_double(b2)) {
          if (bubble_count(b1) != bubble_count(b2)) {
               return 0;
          }

          struct Bubble *h1 = b1.head;
          struct Bubble *h2 = b2.head;

          int value = 1;
          while (0 != value && NULL != h1 && NULL != h2) {
               value = bubble_comparator(*h1, *h2, f);

               h1 = h1->next;
               h2 = h2->next;
          }

          return value;
     }

     return 0;
}

static int
bubble_compare_equals(struct Bubble b1, struct Bubble b2) {
     return (b1.value == b2.value);
}

static int
bubble_compare_lesser(struct Bubble b1, struct Bubble b2) {
     return (b1.value < b2.value);
}

static int
bubble_compare_greater(struct Bubble b1, struct Bubble b2) {
     return (b1.value > b2.value);
}

int
bubble_equals(struct Bubble b1, struct Bubble b2) {
     return bubble_comparator(b1, b2, bubble_compare_equals);
}

int
bubble_lessers(struct Bubble b1, struct Bubble b2) {
     return bubble_comparator(b1, b2, bubble_compare_lesser);
}

int
bubble_greaters(struct Bubble b1, struct Bubble b2) {
     return bubble_comparator(b1, b2, bubble_compare_greater);
}

int
bubble_zero(struct Bubble bubble) {
     if (0 != bubble_double(bubble)) {
          return 0;
     }

     return (0 == bubble.value);
}
