#include "config.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "eval.h"

#define abort_when(c) do{if(c){fprintf(stderr,"at %d\n",__LINE__);return 1;}}while(0)

int
main(int argc, char *argv[]) {
     int ostdout = 0;
     int pipefd[2] = { 0 };
     char outbuf[1024] = { 0 };

     ostdout = dup(STDOUT_FILENO);
     abort_when(0 != pipe(pipefd));
     dup2(pipefd[1], STDOUT_FILENO);
     close(pipefd[1]);

     struct Abyss abyss = { 0 };
     struct Bubble bubble = { 0 };
     struct EvalResult result = { 0 };

     bubble = bubble_wrap(0);
     abyss = abyss_push(abyss, bubble);
     result = eval_prn(abyss, 0);
     abort_when(EVAL_OK != result.code);

     read(pipefd[0], outbuf, 1024);
     abort_when(0x41 != outbuf[0]);

     bubble = bubble_wrap(1);
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_merge(abyss);
     bubble = bubble_wrap(0);
     abyss = abyss_push(abyss, bubble);
     abyss = abyss_merge(abyss);

     result = eval_prn(abyss, 0);
     abort_when(EVAL_OK != result.code);

     read(pipefd[0], outbuf, 1024);
     abort_when(0 != memcmp("AWA", outbuf, 3));

     close(pipefd[0]);
     abyss = abyss_drop(abyss);

     return 0;
}
