CC      := gcc
CXX     := g++
LD      := g++
YACC    := bison
CPPFLAGS:= -O2 -D_BSD_SOURCE -D__EXTENSIONS__
CXXFLAGS:=
LDFLAGS := -g3
LIBS    :=

TARGET  := elr
PTARGET := elr-p
SRC     := main.cc grammar.cc grammar2.cc parser.cc lex.cc \
           lr.cc lalr.cc lalrk.cc dfa.cc dfa2.cc nfa.cc conflict.cc \
           codegen.cc c_codegen.cc \
           bitset.cc equiv.cc stringset2.cc combvect.cc
OBJ     := $(SRC:%.cc=%.o)
POBJ	:= $(SRC:%.cc=%.po)

all: $(TARGET)

depend: .depend

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

$(PTARGET): $(POBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS) -pg

genchcls: symbolset.o genchcls.o
	$(LD) $(LDFLAGS) $^ -o $@

parser.cc parser.h: parser.y
	$(YACC) -t -k -v -o parser.cc $+
	mv -f parser.hh parser.h

template.o: template.cc
	$(CXX) $(CPPFLAGS) -c $< -o $@

charcls.h: genchcls
	./genchcls > $@

clean:
	rm -f $(TARGET) genchcls parser.cc parser.h parser.cc.output *.o *.po *~

realclean: clean
	rm -f charcls.h .depend

.depend: $(SRC)
	$(CC) -MM $+ > $@
include .depend

%.po: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -pg -c $^ -o $@
