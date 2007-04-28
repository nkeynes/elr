/*
 * language.h - the Language class is a kind of container to hold all the
 *              information, structures, etc, relevant to the language being
 *              processed
 */

#ifndef elr_language_H
#define elr_language_H

#include "grammar.h"
#include "fsa.h"
#include "lr.h"
#include "equiv.h"

class Language {
  public:
    Grammar *g;  /* Base grammar, as read from input file */
    LRTable *lr; /* Generated LR automaton */
    NFA *nfa;    /* Generated intermediate NFA */
    DFA *dfa;    /* Generated DFA scanner automaton */
    EquivClassSet *ecs; /* Set of character classes used by dfa/nfa */
};


#endif /* !elr_language_H */
