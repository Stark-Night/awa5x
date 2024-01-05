bin_PROGRAMS = awa5 awa5c
check_PROGRAMS = test_utf8 test_opcodes
noinst_LIBRARIES = libutf8.a libopcodes.a

awa5_SOURCES = awa5.c
awa5_LDADD = libopcodes.a

awa5c_SOURCES = awa5c.c
awa5c_LDADD = libutf8.a libopcodes.a

libutf8_a_SOURCES = utf8.c
libopcodes_a_SOURCES = opcodes.c

test_utf8_SOURCES = test_utf8.c
test_utf8_LDADD = libutf8.a

test_opcodes_SOURCES = test_opcodes.c
test_opcodes_LDADD = libopcodes.a

TESTS = $(check_PROGRAMS)

.PHONY: check-syntax
check-syntax:
	$(AM_V_CC)$(COMPILE) -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wdouble-promotion -Wconversion -Wno-sign-conversion -fsyntax-only $(CHK_SOURCES)
