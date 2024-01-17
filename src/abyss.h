#ifndef ABYSS_H
#define ABYSS_H

#include <stdint.h>

struct Bubble {
     int8_t value;
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

struct Bubble
bubble_wrap(int8_t value);

int
bubble_double(struct Bubble bubble);

#endif
