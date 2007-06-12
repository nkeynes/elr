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
 *
 * As a special exception, you may create a larger work that contains
 * part or all of this parser skeleton and distribute that work
 * under terms of your choice, so long as that work isn't itself a
 * parser generator using the skeleton or a modified version thereof
 * as a parser skeleton.  Alternatively, if you modify or redistribute
 * the parser skeleton itself, you may (at your option) remove this
 * special exception, which will cause the skeleton and the resulting
 * Bison output files to be licensed under the GNU General Public
 * License without this special exception.                       
 */

${START_CODE}
${PARSER_INTERFACE}

class ${PARSER_NAME} {
 public:
    ${PARSER_CONTEXT}

    $PARSER_RETURN_TYPE parse_file( const string *filename );
    $PARSER_RETURN_TYPE parse_file( const string &filename );
    $PARSER_RETURN_TYPE parse_file( const char *filename );
    $PARSER_RETURN_TYPE parse_buffer( char *buf, int len );
    $PARSER_RETURN_TYPE parse_stream( int fd );

 protected:
    $PARSER_RETURN_TYPE parse( struct yy_parseable &yyf );
};

