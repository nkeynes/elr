#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#pragma implementation "nfa.h"
#include "config.h"
#include "nfa.h"
#include "equiv.h"
#include "grammar.h"

/******************************* Constructors ********************************/

NFA::NFA( int css )
{
    numEquivs = css;
    states.push_back( NFAState(numEquivs) ); /* Null state */
    equivs = NULL;
}

NFA *NFA::fromString( string &s, Symbol *sym )
{
    NFA *fsa = new NFA();
    fsa->genStringNFA( s, sym );
    return fsa;
}

NFA *NFA::fromRegexp( string &s, Symbol *sym )
{
    NFA *fsa = new NFA();
    fsa->genRegexpNFA( s, sym );
    return fsa;
}

NFA *NFA::fromEmpty( )
{
    NFA *fsa = new NFA(CHARSET_SIZE);
    fsa->addState();  /* Add start state */
    return fsa;
}

/* Construct a simple NFA for the eof transition */
NFA *NFA::fromEOF( Symbol *sym )
{
    NFA *fsa = new NFA();
    int start = fsa->addState();
    int end = fsa->addState();
    fsa->states[start].addCharMove(EOF_CHAR, end);
    fsa->states[end].accept = sym;
    return fsa;
}


/***************************** NFA Generation ********************************/

/* Return the appropriate unbackslashification of the first element of the
   given string. Update *s to point to the last character of the element */
char unbackslashify( char **s )
{
    /* Azsume (*s)[-1] == '\\' */
    switch( **s ) {
        case 'b': return '\b';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case 'x': /* Hex code (\xFF) */
            if( !isxdigit( (*s)[1] ) ) return 'x';
            /* WRITEME */
            
    }
    return **s;
}

void NFA::genRegexpNFA( string &str, Symbol *sym )
{
    int start, end;
    char *s = (char *)str.c_str();

    start = addState(); /* == state 1 */
    end = parseRegexpString(&s, start);
    states[end].accept = sym;
}

void NFA::genStringNFA( string &str, Symbol *sym )
{
    int start, cur, last;
    char *s = (char *)str.c_str();

    /* actually generates a DFA (trivial for this case) */
    last = cur = start = addState();
    while( *s ) {
        cur = addState();
        if( *s == '\\' ) {
            s++;
            states[last].addCharMove( unbackslashify( &s ), cur );
            if( *s ) s++;
        }
        else states[last].addCharMove( *s++, cur );
        last = cur;
    }
    states[cur].accept = sym;
}

int NFA::parseRegexpString( char **s, int start )
{
    int end, next, substart;
    bool brackets = false;
    end = addState();
    if( **s == '('){
         (*s)++;
         brackets = true;
    }
    while( 1 ) {
        substart = addState();
        states[start].addEmptyMove( substart );
        next = parseRegexpChoice(s, substart);
        states[next].addEmptyMove( end );

        switch( **s ) {
            case '|':
                (*s)++;
                break;
            case ')':
                return end;
            case '\0':
                if( brackets )
                    WHINGE("mismatched parens in regular expression");
                return end;
            default:
                /* Can't happen */
                BUG("Unreachable code branch\n");
        }
    }
}   

int NFA::parseRegexpChoice( char **s, int start )
{
    int cur = start, last = 0, next;
    char c;
    
    while( **s  ) {
        c = **s;
        switch( c ) {
            case '[':
                next = addState();
                states[cur].addClassMove( s, next );
                last = cur;
                cur = next;
                break;
            case '(':
                next = parseRegexpString( s, cur );
                last = cur;
                cur = next;
                break;
            case '*':
                states[last].addEmptyMove( cur );
                /* Fallthrough */
            case '+':
                states[cur].addEmptyMove( last );
                next = addState();
                states[cur].addEmptyMove( next );
                last = cur; cur = next;
                break;
            case '?':
                states[last].addEmptyMove( cur );
                break;
            case '|':
            case ')':
                return cur;
            case '\\':
                (*s)++;
                c = unbackslashify(s);
            default:
                next = addState();
                states[cur].addCharMove( c, next );
                last = cur;
                cur = next;
        }
        (*s)++;
    }
    return cur;
}

void NFA::addAlternation( NFA *fsa )
{
    int start = states.size()-1;
    states[1].addEmptyMove( start+1 );
    
    for( vector<NFAState>::iterator i = fsa->states.begin()+1;
         i != fsa->states.end(); i++ ) {
        int state = addState();
        /* Fixup state numbers */
        states[state].dest = (i->dest == 0 ? 0 : i->dest + start);
        states[state].on = i->on;
        states[state].accept = i->accept;
        for( vector<int>::iterator j = i->epsilonMoves.begin();
             j != i->epsilonMoves.end(); j++ ) {
            states[state].epsilonMoves.push_back(*j + start);
        }
    }
}

/***************************** State functions *******************************/

int NFA::addState( void )
{
    int num = states.size();
    states.push_back( NFAState(numEquivs) );
    return num;
}

void NFAState::addEmptyMove( int dest )
{
    epsilonMoves.push_back(dest);
}

/* Assumption: there is at most one non-empty transition from any NFA state */
void NFAState::addCharMove( int ch, int to )
{
    assert( dest == 0 );
    on.set(ch);
    dest = to;
}

void NFAState::addClassMove( char **s, int to )
{
    int c, c2, n;
    bool invert = false;

    assert( s && *s && **s == '[' && dest == 0 );
    
    (*s)++;
    if( **s == '^' ){ invert = true; (*s)++; }
    if( **s == ']' ){ on.set(']'); (*s)++; }
    
    while( **s && **s != ']' ) {
        switch( **s ) {
            case '[':
                break;
            default:
                if( **s == '\\' ) {
                    (*s)++;
                    c = unbackslashify( s );
                } else c = **s;
                if( (*s)[1] == '-' && (*s)[2] != ']' ) {
                    (*s)+=2;
                    if( **s == '\\' ) {
                        (*s)++;
                        c2 = unbackslashify( s );
                    } else c2 = **s;
                    for( ; c < c2; c++ )
                        on.set(c);
                }
                on.set(c);
        }
        (*s)++;
    }
    if( **s != ']' )
        WHINGE( "Unexpected end of character class in regexp\n" );
    if( invert ) on.toggle();
    dest = to;
}

/****************************** Miscellaneous ********************************/

void NFA::epsilonClosure( int state, set<int> &in )
{
    vector<int>::iterator i;
    for( i = states[state].epsilonMoves.begin();
         i != states[state].epsilonMoves.end(); i++ ) {
        if( in.find(*i) == in.end() ) {
            in.insert(*i);
            epsilonClosure( *i, in );
        }
    }
}

void NFA::move( set<int> &from, int ch, set<int> &to )
{
    set<int>::iterator i;
    /* Assume: each nfa state has at most 1 non-epsilon move */
    for( i = from.begin(); i != from.end(); i++ ) {
        if( states[*i].dest > 0 && states[*i].on.get(ch) ) {
            to.insert( states[*i].dest );
            epsilonClosure( states[*i].dest, to );
        }
    }
}

/*
 * For the given fsa (usually an NFA), compute the sets of equivalence classes.
 * This should not be called more than once as it alters the automata (changes
 * moves to use the equivs)
 */
void NFA::computeEquivClasses(void)
{
    equivs = new EquivClassSet();

    /* Break up equivalence classes from moves */
    FOR_EACH( i, vector<NFAState>, states ) {
        if( i->dest > 0 )
            equivs->addCharClass( i->on );
    }
    numEquivs = equivs->getNumClasses();

    printf( "Equivalence classes: %d\n", numEquivs );
//    equivs->printClassSet();
    
    /* Update the moves to use the classes */
    FOR_EACH( i, vector<NFAState>, states ) {
        if( i->dest > 0 )
            equivs->convertToEquivSet( i->on );
    }
}

/******************************** Text Output ********************************/

void NFA::print( FILE *f )
{
    unsigned int i,j;

    for( i = 1; i < states.size(); i++ ) {
        if( states[i].accept == NULL )
            fprintf( f, "(%d) ", i );
        else {
            fprintf( f, "[%d] {%s}  ", i, states[i].accept->name->c_str() );
        }
        fprintf( f, ": " );
        FOR_EACH( m, vector<int>, states[i].epsilonMoves ) {
            printf( "e -> %d ", *m );
        }
        if( states[i].dest ) {
            if( equivs ) equivs->printClass( states[i].on );
            else states[i].on.print();
            printf( " -> %d ", states[i].dest );
        }
        fprintf( f, "\n" );
    }

    if( equivs ) {
        fprintf( f, "Equivalence classes: %d\n", numEquivs );
        equivs->printClassSet();
        fprintf( f, "\n" );
    }
}
