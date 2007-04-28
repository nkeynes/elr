int yyparse_snarf_file( char *filename );

int main(int argc, char *argv[] )
{
	if( argc > 1 ) {
	  printf( "Result: %d\n", yyparse_snarf_file( argv[1] ) );
	}
}
