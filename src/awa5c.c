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
#include <inttypes.h>
#include <stdint.h>
#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

#include "filemap.h"
#include "utf8.h"
#include "grow.h"
#include "gap.h"
#include "hash.h"
#include "opcodes.h"

struct Status {
     int preamble;
     int comment;
     int label;
     int label_use;
     int text_label_open;
     int text_label_close;
     int text_label_store;
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
     struct Hash text_labels;
     struct GapBuffer output;
     uint32_t outcursor;
     struct GrowBuffer text_parts;
     struct GrowBuffer label_intervals;
};

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     struct FileMap fmap = file_map_open(argv[1]);
     if (FILE_MAP_INVALID == fmap.status) {
          fprintf(stderr, "file map failed\n");
          return 1;
     }

     struct Status cstatus = { 0 };
     cstatus.preamble = 3;

     struct Value cvalue = { 0 };
     cvalue.target = 5;

     struct Buffers cbuffers = { 0 };

     off_t cursor = 0;
     while (cursor < fmap.size) {
          struct UTF8Result decoded = utf8_decode(fmap.buffer + cursor);

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

          if (IS_OB(cstatus.parts[0])) {
               // start text label processing
               cstatus.text_label_open = 1;
               cvalue.target = 8;
               cstatus.parts_cursor = 0;
          } else if (1 == cstatus.text_label_open && IS_CB(cstatus.parts[0])) {
               cstatus.text_label_open = 0;
               cstatus.text_label_close = 1;
               cstatus.parts_cursor = 0;
          } else if (IS_S(cstatus.parts[0]) && cstatus.parts_cursor >= 4) {
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

                    cbuffers.labels[value] = cbuffers.labels[1000];
                    cbuffers.labels[value + 256] = 1;
               } else if (1 == cstatus.label_use && 8 == cvalue.target) {
                    // store label informations to be used later, when
                    // all the code has been processed.
                    cstatus.label_use = 0;

                    uint32_t s = 1;

                    cbuffers.label_intervals = append_buffer(cbuffers.label_intervals,
                                                             &cbuffers.outcursor,
                                                             sizeof(uint32_t));
                    cbuffers.label_intervals = append_buffer(cbuffers.label_intervals,
                                                             &s,
                                                             sizeof(uint32_t));
                    cbuffers.label_intervals = append_buffer(cbuffers.label_intervals,
                                                             &value,
                                                             1);

                    cbuffers.outcursor = cbuffers.outcursor + 4;
               } else if (0 == cstatus.text_label_store && 5 == cvalue.target && TLB == value) {
                    // like the LBL case, store the label destination first.
                    cstatus.text_label_store = 1;

                    cbuffers.labels[1001] = cbuffers.outcursor;
               } else if (1 == cstatus.text_label_open && 8 == cvalue.target) {
                    // store the value as a byte of the label name
                    cbuffers.text_parts = append_buffer(cbuffers.text_parts, &value, 1);
               } else if (1 == cstatus.text_label_close && 1 == cstatus.text_label_store && 8 == cvalue.target) {
                    // now associate the destination with the name of the label.
                    cstatus.text_label_close = 0;
                    cstatus.text_label_store = 0;

                    cbuffers.text_parts = append_buffer(cbuffers.text_parts, "\0", 1);

                    cbuffers.text_labels = hash_insert(cbuffers.text_labels,
                                                       cbuffers.text_parts.bytes,
                                                       cbuffers.text_parts.capacity,
                                                       cbuffers.labels[1001]);
                    cbuffers.text_parts = reset_buffer(cbuffers.text_parts);
               } else {
                    // save opcode to be output later.
                    cbuffers.output = gap_append(cbuffers.output, &value, 1);
                    cbuffers.outcursor = cbuffers.outcursor + 1;

                    if (JMP == value) {
                         cstatus.label_use = 1;
                    }
               }

               cvalue.bits = 0;
               cvalue.value = 0;
               // reminder: parameters can look like opcodes so keep the 5 == target
               cvalue.target = ((5 == cvalue.target && opcode_has_parameter(value))
                                || 1 == cstatus.text_label_open) ?
                    8 :
                    5;
          }

          cursor = cursor + decoded.bytes;
     }

     // we can close these resources since we're done with them.
     file_map_close(&fmap);

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

     // write size of code segment.
     // the second value is planned to be a flag, but at the time of
     // this commit we don't have it ready yet.
     uint32_t coden[2] = {
          htonl((uint32_t)finalsize),
          0,
     };
     fwrite(coden, sizeof(uint32_t), 2, stdout);

     for (size_t i=0; i<cbuffers.label_intervals.capacity;) {
          uint32_t position = 0;
          memcpy(&position, cbuffers.label_intervals.bytes + i, 4);
          i = i + 4;

          uint32_t s = 0;
          memcpy(&s, cbuffers.label_intervals.bytes + i, 4);
          i = i + 4;

          uint32_t value = 0;
          memcpy(&value, cbuffers.label_intervals.bytes + i, s);
          i = i + s;

          uint32_t address = ntohl(cbuffers.labels[value]);

          cbuffers.output = gap_move(cbuffers.output, position);
          cbuffers.output = gap_append(cbuffers.output,
                                       &address,
                                       sizeof(uint32_t));
     }

     // write the actual generated code.
     cbuffers.output = gapwrite(cbuffers.output, cbuffers.outcursor, stdout);

     // pad the output with TRM to align code.
     for (uint64_t i=0; i<finalsize-cbuffers.outcursor; ++i) {
          int8_t trm = TRM;
          fwrite(&trm, 1, 1, stdout);
     }

     fflush(stdout);

     cbuffers.output = gap_shrink(cbuffers.output);
     cbuffers.text_parts = shrink_buffer(cbuffers.text_parts);
     cbuffers.label_intervals = shrink_buffer(cbuffers.label_intervals);
     cbuffers.text_labels = hash_close(cbuffers.text_labels);

     return 0;
}
