%defines "src/bisonparser/parser.h"
%{
    #include <stdio.h> /* for printf and e.g */
    #include <string.h>
    extern int yylex(); /*interface to the handwritten lexer*/
    extern void yyerror(const char *fmt, ...); /*iterface to the handwritten lexer */
    extern char* get_queued_semantic_value();

%}

%union{
    char *semantic_value; // for storing semantic values from stringlit, num or ident
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
%token tk_IMPORT 
%token tk_GOIMPORT 
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
%token <semantic_value> tk_STRINGLIT
%token <semantic_value> tk_NUM 
%token <semantic_value> tk_IDENT 
%token tk_TRUE 
%token tk_FALSE
%token tk_IOTA

%type <semantic_value> var_assign var_assigns var_ident

%%
source: 
    package_stmt 
    top_level_decl 
;

package_stmt: 
    tk_PACKAGE 
    tk_IDENT 
    tk_SEMI 
    { 
        printf("package defined: '%s'\n", get_queued_semantic_value());
    }
;

/* its our main recusrion rule for every infinite global statements and/or declaration */
top_level_decl:
    decl 
    | top_level_decl 
    decl  
;

/* add here any global declaration what you need   */
decl: 
     import_decl 
     | var_decl
;

import_decl:
     import

     | tk_IMPORT 
     tk_LPAREN 
     import_bodys 
     tk_RPAREN
     tk_SEMI 

     | tk_GOIMPORT 
     tk_LPAREN 
     import_bodys 
     tk_RPAREN
     tk_SEMI  
  ;

import:
    /* 
    ---EXAMPLE--- import client "src/clientBase" or import "src/clientBase" ---EXAMPLE---
    */

    tk_IMPORT 
    import_body

    /* 
    ---EXAMPLE--- goimport client "src/clientBase" or import "src/clientBase" ---EXAMPLE---
    */

    |tk_GOIMPORT 
    import_body
        
;

import_bodys:
    import_body

    | import_bodys 
    import_body 
;

import_body:
    tk_IDENT 
    {
        printf("import: alias %s ", get_queued_semantic_value());
    }

    tk_STRINGLIT 
    {
        printf("source %s\n",get_queued_semantic_value());
    }

    | tk_STRINGLIT 
    {
        printf("source %s\n",get_queued_semantic_value());
    }
;

var_decl:
    var 

    | tk_VAR 
    tk_LPAREN
    var_bodys
    tk_RPAREN  
    tk_SEMI 
;

var:
  tk_VAR 
  var_types
  var_body

  | tk_VAR
  var_body 
;

var_types:
    tk_T_STRING 
    {
        printf("var type: string\n");
    }

    |tk_T_BOOL 
    {
        printf("var type: bool\n");
    }

    |tk_T_INT8 
    {
        printf("var type: int8\n");
    }

    |tk_T_UINT8
    {
        printf("var type: uint8");
    }

    |tk_T_BYTE 
    {
        printf("var type: byte\n");
    }

    |tk_T_INT16 
    {
        printf("var type: int16\n");
    }

    |tk_T_UINT16 
    {
        printf("var type: uint16\n");
    }

    |tk_T_INT32
    {
        printf("var type: int32\n");
    }

    |tk_T_UINT32 
    {
        printf("var type: uint32\n");
    }

    |tk_T_RUNE 
    {
        printf("var type: rune\n");
    }

    |tk_T_INT64
    {
        printf("var type: int64\n");
    }

    |tk_T_UINT64
    {
        printf("var type: uint64\n");
    }

    |tk_T_INT
    {
        printf("var type: int\n");
    }

    |tk_T_UINTPTR 
    {
        printf("var type: uintptr");
    }

    |tk_T_FLOAT32
    {
        printf("var type: float32");
    }

    |tk_T_FLOAT64
    {
        printf("var type: float64\n");
    }

    |tk_T_COMPLEX64
    {
        printf("var type: complex64");
    }

    |tk_T_COMPLEX128
    {
        printf("var type: complex128");
    }
;

var_body:
    var_idents
    tk_SEMI 

    |var_idents 
    tk_ASSIGN 
    var_assigns
    tk_SEMI 
;

var_idents:
    var_ident
    {
        printf("var ident: %s\n", $1);
    }

    |var_ident
    {
         printf("var ident: %s, ", $1);
    } 
    tk_COMMA
    var_idents
;

var_ident:
    tk_IDENT 
    {
        $$ = get_queued_semantic_value();
        
    }
;

var_assigns:
    var_assign 
    {
        printf("var value: %s\n", $1);
    }
    | var_assign
    {
        printf("var value: %s, ", $1);
    } 
    tk_COMMA 
    var_assigns
    
;

var_assign:
    tk_NUM 
    {
        {$$ = get_queued_semantic_value();}
        /* printf("var value: %s\n", get_queued_semantic_value()); */
    }
    |tk_STRINGLIT
    {
        {$$ = get_queued_semantic_value();}
       /* printf("var value: %s\n", get_queued_semantic_value()); */
    }
;

var_bodys:
    var_body
    | var_bodys
    var_body 
;