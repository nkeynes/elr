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

#ifndef elr_codegen_h
#define elr_codegen_h 1

#include <stdio.h>
#include "config.h"
#include "grammar.h"
#include "dfa.h"
#include "lr.h"
#include "combvect.h"
#include "conflict.h"

class CodeGen {
public:
    void init( Grammar *g, LRTable *lrt, DFA *fsa, ConflictMap *conf ) {
        grammar = g; dfa = fsa; lr = lrt; conflict = conf;
        parser = new CombVector( lr->states.size(), grammar->numSymbols );
        parser->fromSource(*lr);
        scanner = new CombVector( dfa->states.size(), dfa->numEquivs );
        scanner->fromSource(*dfa);
	computeSymbolUses();
    }
    virtual void createSourceFile( void );
    virtual void createHeaderFile( void );

    static CodeGen *getInstance( language_gen_t lang );
protected:
    Grammar *grammar;
    DFA *dfa;
    LRTable *lr;
    CombVector *parser, *scanner;
    ConflictMap *conflict;
    
    virtual void processFile( const char *inname, const char *outname );
    virtual void handleCommand( char *cmd, int len, FILE *out );
    virtual void writeAttributes( FILE *out );
    virtual void writeIntegerConst( int val, FILE *out );
    virtual void writeIntegerArray( int *arr, int len, FILE *out );
    virtual void writeStringConst( const char *str, FILE *out );
    virtual void writeEscapedString( const char *str, FILE *out );
    virtual void writeIdentifier( const char *str, FILE *out );
    virtual void writeActionCode( const Rule *r,const char *action,FILE *out );
    virtual void writeActionCode( const Terminal *t,const char *action,FILE *out );
    virtual void computeSymbolUses( );
    virtual void computeSymbolUses( Rule *r, const char *action);

/* Subclass responsibility */
    virtual void writeMemberVar( const char *type, const char *name, FILE *out ) = 0;
    virtual void writeSymbolNameArray( FILE *out ) = 0;
    virtual void writeNextCheckArray( CombVector *vect, FILE *out ) = 0;
    virtual void writeParserActions( FILE *out ) = 0;
    virtual void writeLexerActions( FILE *out ) = 0;
    virtual void writeSynthAttrCode( const Symbol *s, FILE *out ) = 0;
    virtual void writeAttrCode( const Rule *r, int n, FILE *out ) = 0;
    virtual void writeAttrCode( const Terminal *s, FILE *out ) = 0;
    virtual char *sourceExt(void) = 0;
    virtual char *headerExt(void) = 0;
    virtual char *sourceSkel(void) = 0;
    virtual char *headerSkel(void) = 0;
};

class C_CodeGen : public CodeGen {
  protected:
    virtual void writeMemberVar( const char *type, const char *name, FILE *out );
    virtual void writeSymbolNameArray( FILE *out );
    virtual void writeStringConst( const char *str, FILE *out );
    virtual void writeNextCheckArray( CombVector *vect, FILE *out );
    virtual void writeParserActions( FILE *out );
    virtual void writeLexerActions( FILE *out );
    virtual void writeSynthAttrCode( const Symbol *r, FILE *out );
    virtual void writeAttrCode( const Rule *r, int n, FILE *out );
    virtual void writeAttrCode( const Terminal *s, FILE *out );
    virtual char *sourceExt(void) { return ".c"; }
    virtual char *headerExt(void) { return ".h"; }
    virtual char *sourceSkel(void) { return "skel.c"; }
    virtual char *headerSkel(void) { return "skel.h"; }
};

#endif /* ! elr_codegen_h */
