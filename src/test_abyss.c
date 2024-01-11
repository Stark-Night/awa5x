#include "config.h"

#include <stdio.h>
#include <string.h>
#include "abyss.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     struct Abyss abyss = { 0 };
     struct Bubble bubble = { 0 };

     bubble = bubble_wrap(8);
     abort_when(8 != bubble.value);

     abyss = abyss_push(abyss, bubble);
     abort_when(NULL == abyss.bubbles);
     abort_when(0 == abyss.size);
     abort_when(0 == abyss.top);

     abort_when(bubble.value != (abyss_top(abyss)).value);

     abyss = abyss_pop(abyss);
     abort_when(0 != abyss.top);

     abyss = abyss_drop(abyss);
     abort_when(NULL != abyss.bubbles);

     return 0;
}
