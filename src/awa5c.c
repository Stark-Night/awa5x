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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "utf8.h"
#include "opcodes.h"

struct Status {
     int preamble;
     int comment;
     int label;
     struct UTF8Result parts[5];
     int parts_cursor;
     uint32_t position;
};

struct Value {
     int value;
     int bits;
     int target;
};

struct Buffers {
     uint32_t labels[1024];
     int8_t *output;
     uint32_t outsize;
     uint32_t outcursor;
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

     struct Buffers cbuffers = { 0 };
     cbuffers.output = malloc(finfo.st_size);
     cbuffers.outsize = (uint32_t)finfo.st_size; // truncates, but it's fine
     if (NULL == cbuffers.output) {
          fprintf(stderr, "malloc failed\n");
          munmap(fmap, finfo.st_size);
          close(fd);
          return 1;
     }

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

          if (cvalue.bits >= cvalue.target) {
               int8_t value = (5 == cvalue.target) ?
                    ((uint8_t)cvalue.value % 32) :
                    (int8_t)cvalue.value;

               if (0 == cstatus.label && 5 == cvalue.target && LBL == value) {
                    // save the label destination.
                    // because we don't yet know the label name, the
                    // value is stored somewhere outside the range of
                    // valid labels.
                    cstatus.label = 1;

                    cbuffers.labels[1000] = cbuffers.outcursor;
               } else if (1 == cstatus.label && 8 == cvalue.target) {
                    // store the label destination where it belongs.
                    cstatus.label = 0;

                    if (0 == cbuffers.labels[value + 256]) {
                         cbuffers.labels[1010] = cbuffers.labels[1010] + 1;
                    }

                    cbuffers.labels[value] = cbuffers.labels[1000];
                    cbuffers.labels[value + 256] = 1;
               } else {
                    // save opcode to be output later, but if for some
                    // reason the generated code is longer than the
                    // input text we abort without graceful shutdowns
                    // because it is something that must never happen.
                    if (cbuffers.outcursor >= cbuffers.outsize) {
                         abort();
                    }

                    cbuffers.output[cbuffers.outcursor] = value;
                    cbuffers.outcursor = cbuffers.outcursor + 1;
               }

               cvalue.bits = 0;
               cvalue.value = 0;
               // reminder: parameters can look like opcodes so keep the 5 == target
               cvalue.target = (5 == cvalue.target && opcode_has_parameter(value)) ? 8 : 5;
          }

          cursor = cursor + decoded.bytes;
     }

     // we can close these resources since we're done with them.
     munmap(fmap, finfo.st_size);
     close(fd);

     if (0 < cstatus.preamble) {
          fprintf(stderr, "not a valid program\n");
          return 1;
     }

     // find the next power of two to align the code segment to.
     // allows for some planned features to be hacked as a data
     // segment after the code.
     uint64_t finalsize = cbuffers.outcursor;
     finalsize = finalsize - 1;
     finalsize = finalsize | finalsize >> 1;
     finalsize = finalsize | finalsize >> 2;
     finalsize = finalsize | finalsize >> 4;
     finalsize = finalsize | finalsize >> 8;
     finalsize = finalsize | finalsize >> 16;
     finalsize = finalsize | finalsize >> 32;
     finalsize = finalsize + 1;

     // write file header for magic detection spells
     int8_t magic[8] = { 0x00, 0x41, 0x57, 0x41, 0x35, 0x30, 0x0D, 0x0A };
     fwrite(magic, 1, 8, stdout);

     // write how many labels are requested.
     // since labels are stored as <jump,index> pairs the number is
     // duplicated to keep things even.
     uint32_t labeln[2] = {
          htonl(cbuffers.labels[1010]),
          htonl(cbuffers.labels[1010]),
     };
     fwrite(labeln, sizeof(uint32_t), 2, stdout);

     // write the labels as <jump,index> pairs.
     for (uint32_t i=0; i<256; ++i) {
          if (0 == cbuffers.labels[i + 256]) {
               continue;
          }

          // keep these temp variables: I met certain compiler
          // optimizations that would change the value or something
          // like that.
          uint32_t offset = htonl(cbuffers.labels[i]);
          uint32_t index = htonl(i);

          uint32_t netorder[2] = { offset, index };
          fwrite(netorder, sizeof(uint32_t), 2, stdout);
     }

     // write size of code segment.
     // the second value is planned to be a flag, but at the time of
     // this commit we don't have it ready yet.
     uint32_t coden[2] = {
          htonl((uint32_t)finalsize),
          htonl((uint32_t)finalsize),
     };
     fwrite(coden, sizeof(uint32_t), 2, stdout);

     // write the actual generated code.
     fwrite(cbuffers.output, 1, cbuffers.outcursor, stdout);

     // pad the output with TRM to align code.
     for (uint64_t i=0; i<finalsize-cbuffers.outcursor; ++i) {
          int8_t trm = TRM;
          fwrite(&trm, 1, 1, stdout);
     }

     fflush(stdout);
     free(cbuffers.output);

     return 0;
}
