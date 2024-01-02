bin_PROGRAMS = awa5 awa5c

awa5_SOURCES = awa5.c

awa5c_SOURCES = awa5c.c

.PHONY: check-syntax
check-syntax:
	$(AM_V_CC)$(COMPILE) -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wdouble-promotion -Wconversion -Wno-sign-conversion -fsyntax-only $(CHK_SOURCES)
