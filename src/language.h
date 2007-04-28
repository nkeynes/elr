/**
 * $Id$
 *
 * Copyright (c) 2000-2007 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
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
