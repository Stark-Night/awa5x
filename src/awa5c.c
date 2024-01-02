#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <stdint.h>

#include "utf8.h"

struct Status {
     int preamble;
     int comment;
     struct UTF8Result parts[5];
     int parts_cursor;
};

struct Value {
     int value;
     int bits;
     int target;
};

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     int fd = open(argv[1], O_RDONLY);
     if (-1 == fd) {
          fprintf(stderr, "no file %s\n", argv[1]);
          return 1;
     }

     struct stat finfo = { 0 };
     if (-1 == fstat(fd, &finfo)) {
          fprintf(stderr, "stat failed\n");
          close(fd);
          return 1;
     }

     if (3 > finfo.st_size) {
          fprintf(stderr, "unrealistic file size %ld\n", (long int)finfo.st_size);
          close(fd);
          return 1;
     }

     void *fmap = mmap(NULL, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
     if (MAP_FAILED == fmap) {
          fprintf(stderr, "mmap failed\n");
          close(fd);
          return 1;
     }

     struct Status cstatus = { 0 };
     cstatus.preamble = 3;

     struct Value cvalue = { 0 };
     cvalue.target = 5;

     off_t cursor = 0;
     while (cursor < finfo.st_size) {
          struct UTF8Result decoded = utf8_decode(fmap + cursor);

          if (0 == cstatus.comment && IS_C(decoded)) {
               cstatus.comment = 1;
          }

          if (1 == cstatus.comment) {
               if (IS_N(decoded)) {
                    cstatus.comment = 0;
               }

               cursor = cursor + decoded.bytes;
               continue;
          }

          if (!IS_A(decoded) && !IS_W(decoded) & !IS_T(decoded) && !IS_S(decoded)) {
               cursor = cursor + decoded.bytes;
               continue;
          }

          if (0 < cstatus.preamble) {
               switch (cstatus.preamble) {
               case 3:
                    cstatus.preamble = cstatus.preamble - ((IS_A(decoded)) ? 1 : 0);
                    break;
               case 2:
                    cstatus.preamble = cstatus.preamble - ((IS_W(decoded)) ? 1 : 0);
                    break;
               case 1:
                    cstatus.preamble = cstatus.preamble - ((IS_A(decoded)) ? 1 : 0);
                    break;
               default:
                    break;
               }

               cursor = cursor + decoded.bytes;
               continue;
          }

          cstatus.parts[cstatus.parts_cursor] = decoded;
          cstatus.parts_cursor = cstatus.parts_cursor + 1;

          if (IS_S(cstatus.parts[0]) && cstatus.parts_cursor >= 4) {
               // maybe awa or ~wa
               if (IS_A(cstatus.parts[1]) && IS_W(cstatus.parts[2]) && IS_A(cstatus.parts[3])) {
                    // awa (= 0)
                    cvalue.value = ((unsigned int)cvalue.value) << 1;
                    cvalue.bits = cvalue.bits + 1;
               } else if (IS_T(cstatus.parts[1]) && IS_W(cstatus.parts[2]) && IS_A(cstatus.parts[3])) {
                    // ~wa (= -1)
                    cvalue.value = -1;
                    cvalue.bits = cvalue.bits + 1;
               } else {
                    // invalid
                    fprintf(stderr,
                            "invalid sequence %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
                            cstatus.parts[1].point,
                            cstatus.parts[2].point,
                            cstatus.parts[3].point);
               }

               // token processed, clear
               cstatus.parts_cursor = 0;
          } else if (IS_W(cstatus.parts[0]) && cstatus.parts_cursor >= 2) {
               // maybe wa
               if (IS_A(cstatus.parts[1])) {
                    cvalue.value = (((unsigned int)cvalue.value) << 1) + 1;
                    cvalue.bits = cvalue.bits + 1;
               } else {
                    // invalid
                    fprintf(stderr,
                            "invalid sequence %" PRIu32 " %" PRIu32 "\n",
                            cstatus.parts[0].point,
                            cstatus.parts[1].point);
               }

               // token processed, clear
               cstatus.parts_cursor = 0;
          } else if (cstatus.parts_cursor >= 4) {
               // invalid
               fprintf(stderr,
                       "invalid sequence %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
                       cstatus.parts[1].point,
                       cstatus.parts[2].point,
                       cstatus.parts[3].point);
               cstatus.parts_cursor = 0;
          } else if (cstatus.parts_cursor > 1 && IS_S(cstatus.parts[cstatus.parts_cursor - 2]) && IS_S(cstatus.parts[cstatus.parts_cursor - 1])) {
               // multiple spaces in a row become only one
               cstatus.parts_cursor = cstatus.parts_cursor - 1;
          }

          cursor = cursor + decoded.bytes;
     }

     if (0 < cstatus.preamble) {
          fprintf(stderr, "not a valid program\n");
          munmap(fmap, finfo.st_size);
          close(fd);
          return 1;
     }

     munmap(fmap, finfo.st_size);
     close(fd);

     return 0;
}
