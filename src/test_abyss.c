#include "config.h"

#include <stdio.h>
#include <string.h>
#include "abyss.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     struct Abyss abyss = { 0 };
     struct Bubble bubble = { 0 };

     abyss = abyss_expand(abyss);
     abort_when(NULL == abyss.bubbles);
     abort_when(NULL == abyss.free);
     abort_when(abyss.free != abyss.bubbles);
     abort_when(abyss.free->next != abyss.bubbles + 1);
     abyss = abyss_drop(abyss);
     abort_when(NULL != abyss.bubbles);

     bubble = bubble_wrap(8);
     abort_when(8 != bubble.value);

     abyss = abyss_push(abyss, bubble);
     abort_when(NULL == abyss.bubbles);
     abort_when(0 == abyss.size);
     abort_when(0 == abyss.used);
     abort_when(abyss.head == abyss.free);
     abort_when(abyss.head != abyss.bubbles);
     abort_when(abyss.free != abyss.bubbles + 1);

     abort_when(bubble.value != (abyss_top(abyss)).value);

     abyss = abyss_pop(abyss);
     abort_when(0 != abyss.used);
     abort_when(NULL != abyss.head);

     abyss = abyss_drop(abyss);
     abort_when(NULL != abyss.bubbles);

     bubble.value = 0;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 1;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 2;
     abyss = abyss_push(abyss, bubble);

     abort_when(abyss.head != abyss.bubbles + 2);

     abyss = abyss_move(abyss, 2);
     abort_when(abyss.head != abyss.bubbles + 1);
     abort_when(1 != abyss.head->value);
     abort_when(2 != abyss.head->next->next->value);

     abyss = abyss_drop(abyss);

     bubble.value = 0;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 1;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_join(abyss, 2);
     abort_when(abyss.bubbles + 2 != abyss.head);
     abort_when(abyss.bubbles + 1 != abyss.head->head);
     abort_when(NULL != abyss.head->next);
     abort_when(abyss.bubbles + 0 != abyss.head->head->next);

     bubble.value = 3;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_join(abyss, 2);
     abort_when(3 != abyss.head->head->value);
     abort_when(NULL == abyss.head->head->next);
     abort_when(NULL == abyss.head->head->next->head);
     abyss = abyss_pop(abyss);
     abort_when(3 != abyss.head->value);
     abort_when(NULL == abyss.head->next->head);

     abyss = abyss_drop(abyss);

     bubble.value = 0;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 1;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_merge(abyss);
     abort_when(NULL == abyss.head->head);
     abort_when(1 != abyss.head->head->value);

     bubble.value = 2;
     abyss = abyss_push(abyss, bubble);
     bubble.value = 3;
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_merge(abyss);
     abyss = abyss_merge(abyss);
     abort_when(3 != abyss.head->head->value);

     abyss = abyss_drop(abyss);

     return 0;
}
