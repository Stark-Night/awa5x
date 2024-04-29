# awa5x - Extended AWA5.0
# Copyright © 2024 Starknights

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

bin_PROGRAMS = awa5 awa5c awa5p awa5d
check_PROGRAMS = test_utf8 test_opcodes test_abyss \
	test_hash test_grow test_gap
noinst_LIBRARIES = libutf8.a libopcodes.a libabyss.a libeval.a \
	libfilemap.a libhash.a libgrow.a libgap.a

EXTRA_DIST = utf8.h opcodes.h abyss.h eval.h filemap.h hash.h grow.h gap.h

awa5_SOURCES = awa5.c
awa5_LDADD = libfilemap.a libopcodes.a libabyss.a libeval.a

awa5c_SOURCES = awa5c.c
awa5c_LDADD = libfilemap.a libutf8.a libgrow.a libhash.a libgap.a libopcodes.a

awa5p_SOURCES = awa5p.c
awa5p_LDADD = libfilemap.a libutf8.a libgrow.a libopcodes.a

awa5d_SOURCES = awa5d.c
awa5d_LDADD = libfilemap.a libopcodes.a libabyss.a libeval.a

if !HAVE_GETLINE
awa5_LDADD += ../vendors/libglgetline.a
awa5d_LDADD += ../vendors/libglgetline.a
endif

if HAVE_WAPI
awa5_LDADD += -lws2_32
awa5c_LDADD += -lws2_32
awa5p_LDADD += -lws2_32
awa5d_LDADD += -lws2_32
endif

libutf8_a_SOURCES = utf8.c
libopcodes_a_SOURCES = opcodes.c
libabyss_a_SOURCES = abyss.c
libeval_a_SOURCES = eval.c
libfilemap_a_SOURCES = filemap.c
libhash_a_SOURCES = hash.c
libgrow_a_SOURCES = grow.c
libgap_a_SOURCES = gap.c

if !HAVE_GETLINE
libeval_a_CFLAGS = -I../vendors/
endif

test_utf8_SOURCES = test_utf8.c
test_utf8_LDADD = libutf8.a

test_opcodes_SOURCES = test_opcodes.c
test_opcodes_LDADD = libopcodes.a

test_abyss_SOURCES = test_abyss.c
test_abyss_LDADD = libabyss.a

test_hash_SOURCES = test_hash.c
test_hash_LDADD = libhash.a

test_grow_SOURCES = test_grow.c
test_grow_LDADD = libgrow.a

test_gap_SOURCES = test_gap.c
test_gap_LDADD = libgap.a

TESTS = $(check_PROGRAMS) test_eval

.PHONY: check-syntax
check-syntax:
	$(AM_V_CC)$(COMPILE) -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wdouble-promotion -Wconversion -Wno-sign-conversion -fsyntax-only $(CHK_SOURCES)
