# awa5x - Extended AWA5.0
# Copyright © 2024 Starknights
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

EXTRA_DIST = main.tex \
hello1.awa hello2.awa hello3.awa \
celsius.awa celsius2.awa celsius3.awa celsius4.awa \
charcount.awa

CLEANFILES = awa5x_book.pdf main.aux main.log main.toc

if HAVE_LATEX
dist_doc_DATA = awa5x_book.pdf

examples = $(shell find . -name '*.awa')

awa5x_book.pdf: $(srcdir)/main.tex $(examples)
	$(AM_V_GEN)$(LATEX) --interaction=batchmode $<
	$(AM_V_GEN)$(LATEX) --interaction=batchmode $<
	$(AM_V_at)mv $(builddir)/main.pdf $(builddir)/awa5x_book.pdf
endif
