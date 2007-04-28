/*
 * lr1.cc - code to resolve inadequate states by moving to full LR(1) if
 *          necessary. Assumes LALR(1) lookahead has already been computed.
 */

#include <vector>
#include <set>
#include "grammar.h"
#include "lr.h"

/*
 * List all the possible derivations which can reach the given state,
 * excluding loops
 */
 LRTable::getDerivations( int state )
{
    
    
}
