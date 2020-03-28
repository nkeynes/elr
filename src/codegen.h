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
#include <map>
#include "config.h"
#include "grammar.h"
#include "dfa.h"
#include "lr.h"
#include "combvect.h"
#include "conflict.h"

#define SCAN_TYPE "yyscan_t"

#define MATCH(a,l,b) (!strncmp(a,b,l) && l == strlen(b))

class CodeGen {
public:
    virtual void init( Grammar *g, LRTable *lrt, DFA *fsa, ConflictMap *conf ) {
        grammar = g; dfa = fsa; lr = lrt; conflict = conf; errors = 0;
        parser = new CombVector( lr->states.size(), grammar->numSymbols );
        parser->fromSource(*lr);
        scanner = new CombVector( dfa->states.size(), dfa->numEquivs );
        scanner->fromSource(*dfa);
	computeTypes();
    }
    virtual void createSourceFile( void );
    virtual void createHeaderFile( void );
    virtual int getErrors();

    static CodeGen *getInstance( language_gen_t lang );
protected:
    Grammar *grammar;
    DFA *dfa;
    LRTable *lr;
    CombVector *parser, *scanner;
    ConflictMap *conflict;
    int errors, outputLine;
    string outputFile;
    unordered_map<string,string> typeNameMap;

    virtual void processFile( const char *inname, const char *outname );
    virtual void handleCommand( char *cmd, int len, FILE *out );
    virtual void writeAttributes( FILE *out );
    virtual void writeIntegerConst( int val, FILE *out );
    virtual void writeIntegerArray( int *arr, int len, FILE *out );
    virtual void writeStringConst( const char *str, FILE *out );
    virtual void writeEscapedString( const char *str, FILE *out );
    virtual void writeIdentifier( const char *str, FILE *out );
    virtual void writeType( const char *str, FILE *out );
    virtual void writeCode( const Action &action,FILE *out );
    virtual void writeActionCode( const Rule *r,const ActionItem &action,FILE *out );
    virtual void writeActionCode( const Terminal *t,const ActionItem &action,FILE *out );
    virtual void computeSymbolUses( Rule *r, const char *action);
    virtual void computeTypes();
    virtual string *computeTypes( map<int,int> &seen, Symbol *s, string *type, int resultUsed );
    virtual string getSkeletonFile( const char *file );
    virtual string getOutputSourceFile( void );
    virtual string getOutputHeaderFile( void );
    virtual void write( const char *str, FILE *out );
    virtual void write( const char *str, int length, FILE *out );

/* Subclass responsibility */
    virtual void writeMemberVar( const char *type, const char *name, FILE *out ) = 0;
    virtual void writeSymbolNameArray( FILE *out ) = 0;
    virtual void writeNextCheckArray( CombVector *vect, FILE *out ) = 0;
    virtual void writeParserActions( FILE *out ) = 0;
    virtual void writeParserReturn( FILE *out ) = 0;
    virtual void writeReturn( FILE *out ) = 0;
    virtual void writeLexerActions( FILE *out ) = 0;
    virtual void writeSynthAttrCode( const Symbol *s, FILE *out ) = 0;
    virtual void writeAttrCode( const Rule *r, int n, FILE *out ) = 0;
    virtual void writeAttrCode( const Terminal *s, FILE *out ) = 0;
    virtual string *defaultSymType() = 0;
    virtual const char *sourceExt(void) = 0;
    virtual const char *headerExt(void) = 0;
    virtual const char *sourceSkel(void) = 0;
    virtual const char *headerSkel(void) = 0;
};

class C_CodeGen : public CodeGen {
    virtual void init( Grammar *g, LRTable *lrt, DFA *fsa, ConflictMap *conf );
  protected:
    vector<int>expectedTokens;
    vector<int>expectedTokenIndex;

    virtual void computeExpectedTokens( void );
    virtual void handleCommand( char *cmd, int len, FILE *out );
    virtual void writeMemberVar( const char *type, const char *name, FILE *out );
    virtual void writeSymbolNameArray( FILE *out );
    virtual void writeStringConst( const char *str, FILE *out );
    virtual void writeNextCheckArray( CombVector *vect, FILE *out );
    virtual void writeParserActions( FILE *out );
    virtual void writeParserReturn( FILE *out );
    virtual void writeReturn( FILE *out );
    virtual void writeLexerActions( FILE *out );
    virtual void writeCode( const Action &action,FILE *out );
    virtual void writeSynthAttrCode( const Symbol *r, FILE *out );
    virtual void writeAttrCode( const Rule *r, int n, FILE *out );
    virtual void writeAttrCode( const Terminal *s, FILE *out );
    virtual void writeType( const char *str, FILE *out );
    virtual string *defaultSymType();
    virtual const char *sourceExt(void) { return ".c"; }
    virtual const char *headerExt(void) { return ".h"; }
    virtual const char *sourceSkel(void) { return "skel.c"; }
    virtual const char *headerSkel(void) { return "skel.h"; }
};

class Cpp_CodeGen : public C_CodeGen {
protected:
    virtual const char *sourceExt(void) { return ".cc"; }
    virtual const char *headerExt(void) { return ".hh"; }
    virtual const char *sourceSkel(void) { return "skel.cc"; }
    virtual const char *headerSkel(void) { return "skel.hh"; }
    
};

#endif /* ! elr_codegen_h */
