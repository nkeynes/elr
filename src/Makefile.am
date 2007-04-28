## Process this file with automake to produce Makefile.in

INCLUDES = \
        -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
        -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
        -Ish4 \
        @PACKAGE_CFLAGS@

bin_PROGRAMS = elr

elr_SOURCES = main.cc elr.h \
              conflict.cc conflict.h dfa2.cc dfa.cc dfa.h equiv.cc equiv.h \
              grammar2.cc grammar.cc grammar.h lalr.cc parser.cc \
              lalrk.cc language.h lex.cc lex.h lr.cc lr.h \
              nfa.cc nfa.h \
              c_codegen.cc codegen.cc codegen.h \
              combvect.cc combvect.h \
	      bitset.cc bitset.h \
              stringset2.cc stringset2.h
              
BUILT_SOURCES = parser.h parser.cc

parser.cc parser.h: parser.y
	bison -t -d -k -o parser.cc $+
	mv -f parser.hh parser.h
	