#include <stdio.h>
int yyparse_file( char *filename );

int main(int argc, char *argv[] )
{
	if( argc > 1 ) {
	  printf( "Result: %d\n", yyparse_file( argv[1] ) );
	}
	return 0;
}
