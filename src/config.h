#ifndef elr_config_H
#define elr_config_H 1

#include <string>
#include <iostream>

using namespace std;

#define INPUT_FILE_EXT "elr"
#define DEBUG_FILE_EXT "out"
#define SKELETON_PATH "skel/"
#define DEFAULT_PARSER_NAME "yyparse"
#define CHARSET_SIZE 257
#define EOF_CHAR 256
#define WHITESPACE_TERM "WHITESPACE"

typedef enum { PG_NORMAL, PG_NONE, PG_EXTERN } parser_gen_t;
typedef enum { SG_NORMAL, SG_CONTEXT, SG_NONE, SG_EXTERN } scanner_gen_t;
typedef enum { PT_LALR = 0, PT_LR = 1, PT_NSLR = 2, PT_GLR = 3 } parser_type_t;
typedef enum { V_TERSE = 0, V_NORMAL, V_VERBOSE, V_DEBUG } verbosity_t;
typedef enum { LANG_C = 0, LANG_CPP, LANG_JAVA, LANG_ADA } language_gen_t;

/* What to output - generated code, lex/yacc input, rex/lalr input */
typedef enum { OUT_GENCODE = 0, OUT_YACC, OUT_LALR } output_gen_t;

class Config {
  public:
    output_gen_t outputGen;
    scanner_gen_t scannerGen;
    parser_gen_t parserGen;
    parser_type_t parserType;
    language_gen_t languageGen;
    int parserLookahead; /* -1 = arbitrary */
    bool genHeader;
    bool genDebug;
    bool verbose;
    string skeletonPath;
    string *outputBase;
    string *outputSourceFile;
    string *outputHeaderFile;
    string *outputDebugFile;
    string *inputFile;

    Config() {
        outputGen = OUT_GENCODE;
        scannerGen = SG_CONTEXT; parserGen = PG_NORMAL; parserType=PT_LALR;
        parserLookahead = 1; languageGen = LANG_C; genHeader = true;
        outputBase= outputSourceFile= outputHeaderFile= outputDebugFile= NULL;
        inputFile = NULL; skeletonPath = string(SKELETON_PATH);
        genDebug = false; verbose=false;
    }
};

extern Config config;

#define BUG( x ) do{ cerr << "BUG: "  x " at " __FILE__ ":" <<  __LINE__ << \
                       " in " << __PRETTY_FUNCTION__  ; abort(); } while(0)
#define WHINGE( x ) cerr << "Error: " x

#endif /* !elr_config_H */
