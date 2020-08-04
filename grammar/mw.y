%defines "src/bisonparser/parser.h"
%{
    #include <stdio.h> /* for printf and e.g */
    #include <string.h>
    extern int yylex(); /*interface to the handwritten lexer*/
    extern void yyerror(const char *fmt, ...); /*iterface to the handwritten lexer */
    extern char* get_queued_semantic_value();
    extern int lookup_symbol_table(char *semantic_value);
    extern int insert_symbol_table(char *semantic_value);
    extern char* text;

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
%nonassoc tk_VAR 
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




%%
source: 
    package_stmt 
    imports 
    decls 
;

package_stmt: 
    tk_PACKAGE 
    tk_IDENT 
    { 
        printf("package name: '%s'\n", get_queued_semantic_value());
    }
;

/* its our main recusrion rule for every infinite global statements and/or declaration */
decls:
    decl 
    | decls
    decl  
;

imports:
    import_decl 
    |imports 
    import_decl 
;


/* add here any global declaration what you need   */
decl: 
      common_decl
      
      |class_decl 
;

import_decl:
     import

     | tk_IMPORT 
     tk_LPAREN 
     import_bodys 
     tk_RPAREN

     | tk_GOIMPORT 
     tk_LPAREN 
     import_bodys 
     tk_RPAREN
 
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
        printf("import: alias '%s', ", get_queued_semantic_value());
    }

    tk_STRINGLIT 
    {
        printf("source '%s'\n",get_queued_semantic_value());
    }

    | tk_STRINGLIT 
    {
        printf("import source '%s'\n",get_queued_semantic_value());
    }
;



common_decl:
    tk_VAR var_decl 

    | tk_VAR 
    tk_LPAREN
    var_decl_list 
    tk_RPAREN
;

var_decl:
    decl_name_list ntype

    |decl_name_list 
    ntype 
    tk_ASSIGN
    expr_list 

    |decl_name_list 
    tk_ASSIGN
    expr_list  
;

var_decl_list:
    var_decl 

    | var_decl_list 
    tk_SEMI 
    var_decl 
;

decl_name:
    tk_IDENT
    {
        printf("decl name: %s ", get_queued_semantic_value());
    }
;

decl_name_list:
    decl_name 

    |decl_name_list 
    tk_COMMA 
    decl_name 
;

expr:
    tk_IDENT
    |tk_STRINGLIT
    |tk_NUM 
;

expr_list:
    expr 

    |expr_list 
    tk_COMMA 
    expr 
;


ntype:
    ptrtype 
    |lookup_symbol_table
    |dotname
    |builtin_type
;

ptrtype:
    tk_MUL 
    ntype 
;

dotname:
    

    name 
    tk_DOT 
    lookup_symbol_table 
;

lookup_symbol_table:
   name
    {
       
            printf("\nsymbol for lookup: '%s'\n", get_queued_semantic_value());
            if (lookup_symbol_table(get_queued_semantic_value())==1){
                printf("\nsymbol found in symbol table: '%s'\n", get_queued_semantic_value()); 
            }else 
             yyerror("undefined user type '%s'\n", get_queued_semantic_value()); 
    }
;



builtin_type:
tk_T_STRING 
   
    |tk_T_BOOL 
    
    |tk_T_INT8 

    |tk_T_UINT8
   
    |tk_T_BYTE 

    |tk_T_INT16 
   
    |tk_T_UINT16 
    
    |tk_T_INT32
    
    |tk_T_UINT32 
    
    |tk_T_RUNE 

    |tk_T_INT64
  
    |tk_T_UINT64
   
    |tk_T_INT
  
    |tk_T_UINTPTR 
  
    |tk_T_FLOAT32
   
    |tk_T_FLOAT64
    
    |tk_T_COMPLEX64
  
    |tk_T_COMPLEX128
;

name:
    tk_IDENT
;

class_decl:

    tk_CLASS 
    tk_IDENT 
    {
        if (insert_symbol_table(get_queued_semantic_value()) == 1)
        {
            printf("\nnew symbol created in symbol table: '%s'\n", get_queued_semantic_value());
        }
    }
    tk_LCBRACKET 
    var_decl_list 
    tk_RCBRACKET

;
