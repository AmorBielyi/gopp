%defines "src/bisonparser/parser.h"
%{
    #include <stdio.h> /* for printf and e.g */
    #include <string.h>
    extern int yylex(); /*interface to the handwritten lexer*/
    extern void yyerror(const char *fmt, ...); /*iterface to the handwritten lexer */
    extern char* get_lookuped_semantic_value_ident();
    extern char* get_lookuped_semantic_value_string();
    extern char* get_lookuped_semantic_value_num();

    
%}

%union{
    char *semantic_value_ident; // for storing semantic values from stringlit, num or ident
    char *semantic_value_string;
    char *semantic_value_num;
}

/*Original keywords*/
%token tk_BREAK
%token tk_CASE
%token tk_CHAN
%token tk_CONST
%token tk_CONTINUE
%token tk_DEFAULT
%token tk_DEFER
%token tk_ELSE
%token tk_FALLTHROUGH
%token tk_FOR 
%token tk_GO 
%token tk_GOTO 
%token tk_IF 
%token tk_IMPORT 
%token tk_INTERFACE 
%token tk_MAP 
%token tk_PACKAGE
%token tk_RANGE 
%token tk_RETURN 
%token tk_SELECT 
%token tk_STRUCT 
%token tk_SWITCH
%token tk_TYPE 
%token tk_VAR 
/*MW's original keywords*/
%token tk_CLASS
%token tk_EXTENDS
%token tk_IMPLEMENTS
%token tk_THIS 
%token tk_NEW 
%token tk_SUPER 
%token tk_PUBLIC 
%token tk_PRIVATE
%token tk_PTRSELECT
%token tk_OVERRIDE
%token tk_VOID 
%token tk_INCLUDE 
%token tk_GOINCLUDE 
/* Original built-in types */
%token tk_T_STRING
%token tk_T_BOOL 
%token tk_T_INT8
%token tk_T_UINT8
%token tk_T_BYTE
%token tk_T_INT16
%token tk_T_UINT16
%token tk_T_INT32
%token tk_T_UINT32
%token tk_T_RUNE
%token tk_T_INT64
%token tk_T_UINT64
%token tk_T_INT 
%token tk_T_UINT 
%token tk_T_UINTPTR
%token tk_T_FLOAT32
%token tk_T_FLOAT64
%token tk_T_COMPLEX64
%token tk_T_COMPLEX128
/*Original operators*/
%token tk_ADD 
%token tk_SUB
%token tk_MUL 
%token tk_DIV
%token tk_MOD 
%token tk_AND 
%token tk_OR 
%token tk_XOR
%token tk_ASSIGN
%token tk_LPAREN
%token tk_RPAREN
%token tk_LSBRACKET
%token tk_RSBRACKET
%token tk_LCBRACKET
%token tk_RCBRACKET
/*Original punctuation*/
%token tk_COMMA
%token tk_DOT
%token tk_SEMI 
%token tk_UNDERSCORE
%token tk_COLON 
/*Original boolean and assigment operators*/
%token tk_LSHIFT
%token tk_RSHIFT
%token tk_EQXOR
%token tk_EQOR
%token tk_EQAND 
%token tk_EQANDXOR 
%token tk_EQRSHIFT
%token tk_EQLSHIFT
%token tk_LOGICAND 
%token tk_LOGICOR 
%token tk_EQADD 
%token tk_EQSUB
%token tk_EQMUL 
%token tk_EQDIV
%token tk_EQMOD 
%token tk_ANDXOR 
/*Original relational operators*/
%token tk_NEG 
%token tk_LSS 
%token tk_GRT 
%token tk_NOTEQ
%token tk_EQ 
%token tk_EQLSS
%token tk_EQGRT 
/*Original misc*/
%token tk_SHORTDECL
%token tk_ARROW 
%token tk_INC 
%token tk_DEC 
%token tk_ELLIPSIS
%token <semantic_value_string> tk_STRINGLIT
%token <semantic_value_num> tk_NUM 
%token <semantic_value_ident> tk_IDENT 
%token tk_TRUE 
%token tk_FALSE


%%

source: {yyerror("Source can't be empty; expected package statement");}

    /* !!!!!! MUST_BE_DELETED this is test rule for testing lookuped semantic values !!!!!! MUST_BE_DELETED*/
    
    | tk_CLASS tk_IDENT {printf("class name: %s ", get_lookuped_semantic_value_ident());}
      tk_EXTENDS tk_IDENT {printf("class extends: %s ", get_lookuped_semantic_value_ident());}
      tk_IMPLEMENTS tk_STRINGLIT {printf("class implements: %s ", get_lookuped_semantic_value_string());}
      tk_STRINGLIT {printf("ext: %s ", get_lookuped_semantic_value_string());}


    | package_stmt {yyerror("expected ';'");}
    | package_stmt tk_SEMI  {yyerror("unrecongnized symbol or expected declaration");}
    | package_stmt tk_SEMI 
      top_level_decl 
;


package_stmt: 
    tk_PACKAGE tk_IDENT {
        //printf("lookuped: %s", get_lookuped_semantic_value());
        if (strcmp(get_lookuped_semantic_value_ident(), "_") == 0)
            yyerror("package name can't be only '_'");
        printf("package defined: '%s'\n", get_lookuped_semantic_value_ident());
    }
    | tk_PACKAGE tk_NUM {yyerror("pakcage name can't be integer");}
    | tk_PACKAGE tk_STRINGLIT {yyerror("package name can't be string");}
;

top_level_decl:
    decl 
    | top_level_decl decl  /* its our main recusrion rule for every infinite global statements and/or declaration */
;

decl: /* add here any global declaration what you need   */
     include_decl  
    | class_decl
;

include_decl:
     include
  ;

include:
    tk_INCLUDE tk_IDENT{printf("include source: alias %s ", get_lookuped_semantic_value_ident());}
    tk_STRINGLIT {printf(" source %s\n",get_lookuped_semantic_value_string());}
    
    | tk_GOINCLUDE tk_IDENT{printf("go include source: alias %s ", get_lookuped_semantic_value_ident());}
    tk_STRINGLIT {printf(" source %s\n",get_lookuped_semantic_value_string());}

    | tk_INCLUDE tk_STRINGLIT  {printf("include source defined: '%s'\n", get_lookuped_semantic_value_string());}
    | tk_GOINCLUDE tk_STRINGLIT {printf("go include source defined : '%s'\n", get_lookuped_semantic_value_string());}
    | tk_GOINCLUDE tk_NUM {yyerror("go include source can't be integer");}
    | tk_INCLUDE tk_NUM {yyerror("include source can't be integer");}
    
;






class_decl:
    class 
 ;

 class:
    tk_CLASS
;












/*
package_decl:
   package
   | package_decl package
;

package:
    tk_PACKAGE tk_IDENT tk_SEMI
;

*/