## Process this file with automake to produce Makefile.in

INCLUDES = \
        -DSKELETON_DIR=\""$(DESTDIR)$(datadir)/$(PACKAGE)/skel"\" \
        -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
        @PACKAGE_CFLAGS@

bin_PROGRAMS = elr

check_PROGRAMS = unittest

elr_SOURCES = main.cc elr.h \
              conflict.cc conflict.h dfa2.cc dfa.cc dfa.h equiv.cc equiv.h \
              grammar2.cc grammar.cc grammar.h lalr.cc parser.cc \
              lalrk.cc language.h lex.cc lex.h lr.cc lr.h \
              nfa.cc nfa.h \
              c_codegen.cc codegen.cc codegen.h \
              combvect.cc combvect.h \
	      bitset.cc bitset.h \
              stringset2.cc stringset2.h
              
unittest_SOURCES = unittest.cc testlalr.cc parser.cc parser.h \
             grammar2.cc grammar.cc grammar.h lalr.cc lex.cc lex.h \
             lr.cc lr.h config.h nfa.cc nfa.h dfa.cc dfa2.cc dfa.h \
             conflict.cc conflict.h \
             bitset.cc bitset.h stringset2.cc stringset2.h equiv.h equiv.cc

unittest_LDFLAGS = $(CPPUNIT_LIBS) $(LIBADD_DL)
            
BUILT_SOURCES = parser.h parser.cc

parser.cc parser.h: parser.y
	bison -t -d -k -o parser.cc $+
	mv -f parser.hh parser.h

TESTS = $(check_PROGRAMS)