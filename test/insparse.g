/**
 * Grammar for a simple machine architecture specification language. Included
 * for regression purposes as it triggered a couple of bugs in 0.5.2
 */

%casesensitive off;
%name InstructionParser;

start: flags register_block rule_list 
       ;

flags: flags '%littleendian'
     | flags '%bigendian' 
     | flags '%opcodesize' '=' NUMBER
     | flags '%name' '=' "\"[^\"]*\"" 
     | 			
     ;

register_block: 'registers' '{' register_list '}';

register_list: register_list register ';'
	       | register ';'
	       ;

register: type_name var_name_list '=' reg_list opt_overlap_list
	| type_name var_name_list	
	;

type_name: IDENTIFIER	
	| IDENTIFIER '[' NUMBER ']'	
	;

var_name_list: var_name_list ',' IDENTIFIER
	       | IDENTIFIER		
	       ;

reg_list: reg_list ',' IDENTIFIER '..' IDENTIFIER
        | reg_list ',' IDENTIFIER		
        | IDENTIFIER '..' IDENTIFIER		
        | IDENTIFIER				
        ;

opt_overlap_list: 'overlaps' reg_list
          | 'overlaps' reg_list	'swapped' 	
	  |					
	  ;

rule_list: rule_list rule NL  
	   | rule NL	
	   ;
rule: bitpattern INSTRUCTION;

bitpattern: 
	    bitpattern "[01?]+"			
          | bitpattern "0x[0-9a-fA-F]+" 	
          | bitpattern bitoperand 		
	  | bitpattern '{' INSTRUCTION '}'	
          | "[01?]+"				
	  | "0x[0-9a-fA-F]+"			
	  | bitoperand				
	  | '{' INSTRUCTION '}'      		
          ;

bitoperand: '(' IDENTIFIER ':' NUMBER opt_signed use_mode opt_shift ')';

opt_signed: 's'
                 | 'u' 
                 | 
                 ;
opt_shift: '<<' NUMBER
               | 
               ;

use_mode: '+' 
        | '=' 
        |
        ;

NUMBER = "[0-9]+";

IDENTIFIER = "[A-Za-z_][A-Za-z0-9_]*";
INSTRUCTION = "[A-Za-z][^\n{}]+";
NL = "\n";
WHITESPACE = "[ \t\r\n]+|##[^\n]*";
