
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

/* Subclass responsibility */
    virtual void writeMemberVar( const char *type, const char *name, FILE *out ) = 0;
    virtual void writeSymbolNameArray( FILE *out ) = 0;
    virtual void writeNextCheckArray( CombVector *vect, FILE *out ) = 0;
    virtual void writeParserActions( FILE *out ) = 0;
    virtual void writeLexerActions( FILE *out ) = 0;
    virtual void writeSynthAttrCode( const Rule *r, FILE *out ) = 0;
    virtual void writeAttrCode( const Rule *r, int n, FILE *out ) = 0;
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
    virtual void writeSynthAttrCode( const Rule *r, FILE *out );
    virtual void writeAttrCode( const Rule *r, int n, FILE *out );
    virtual char *sourceExt(void) { return ".c"; }
    virtual char *headerExt(void) { return ".h"; }
    virtual char *sourceSkel(void) { return "skel.c"; }
    virtual char *headerSkel(void) { return "skel.h"; }
};

#endif /* ! elr_codegen_h */
