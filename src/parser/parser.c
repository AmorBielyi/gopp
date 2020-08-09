#include <stdio.h>
#include <string.h>
#include "../lexer/lexer.h"

token_type token;
void parse_grammar();
void parse_import_decl_without_alias(char *);
void parse_import_decl_with_alias(char* );
void parse_import_decl_list();
void parse_var_decl_name_list();
void parse_var_decl_expr_list();
void parse_var_decl_expr();
void parse_builtin_type();
int is_builtin_type(token_type token);

/* PACKAGE RULE BEGIN */
void parse_top_package_stmt()
{
    token = gpplex();
    if(token == tk_IDENT)
    {
        printf("package name: %s\n", get_queued_semantic_value());
        token = gpplex();
    }  
    else
        gpperror(1,"expected package name");
}
/* PACKAGE RULE END */

int from_scope = 0;
/* IMPORTS RULE BEGIN */
void parse_top_import_decl()
{
    if (from_scope != 1){
        token = gpplex();
    }

    
    switch(token){
        case tk_STRINGLIT:
            parse_import_decl_without_alias(get_queued_semantic_value());
           break;
        case !tk_STRINGLIT:
            gpperror(1,"expected import source");
        case tk_IDENT:
            parse_import_decl_with_alias(get_queued_semantic_value());
            break;
        case tk_LPAREN:
            token = gpplex();
            from_scope = 1;
            parse_import_decl_list();
            from_scope = 0;
            if (token == tk_RPAREN){
                 token = gpplex();
                 break;
             }else{
                 gpperror(1,"expected ')' after import scope\n");
             }
    }
        
}

void parse_import_decl_list(){
    do {
         parse_top_import_decl();
    }while(token == tk_IDENT || token == tk_STRINGLIT);
}

void parse_import_decl_without_alias(char* semantic_value_source_name)
{
        printf("import source '%s'\n", semantic_value_source_name);
        token = gpplex();
       // parse_grammar();
}

void parse_import_decl_with_alias(char *semantic_value_alias_name)
{
    printf("import: alias '%s' ", semantic_value_alias_name);
    token = gpplex();
    if (token == tk_STRINGLIT)
    {
        printf("source '%s'\n",get_queued_semantic_value());
        token = gpplex();
        //parse_grammar();
    }else
        gpperror(1,"expected package source after alias");
        
}
/* IMPORTS RULE END */
int is_var_decl = 0;
int is_pointer_type = 0 ;
/* VAR RULE BEGIN */
void parse_var_decl_name(){
    if (from_scope != 1) {
        token = gpplex();
    }
    switch(token){
       // int is_pointer_type = 0;
      
            case tk_IDENT:
             printf("var: name '%s' ", get_queued_semantic_value());
             token = gpplex();
           
    
             if (token == tk_MUL){
                 is_pointer_type = 1;
                 token = gpplex();
                 //printf("token type %u\n", token);
                 if (token != tk_IDENT && is_builtin_type(token) != 1)
                 {
                     gpperror(1,"expected type name after pointer symbol '*'");
                 } 
             }
             
             parse_builtin_type();
            /* 
                for const declaration only 
            */
             if (is_var_decl == 0){ 
                 if(token == tk_IDENT){
                     gpperror(1,"invalid constant type %s", get_queued_semantic_value());
                 }
             }

            /* 
                for var declaration only 
            */
            if(is_var_decl == 1){  
             if(token == tk_IDENT){
                 char *semantic_value_qualifiedpackage_or_type = _strdup(get_queued_semantic_value());
                // printf("var: type '%s' ", get_queued_semantic_value());
                 token = gpplex();
                 if (token == tk_DOT){
                     token = gpplex();
                     if (token == tk_IDENT){
                         printf("var: type (qualified) '%s' from package '%s' ", get_queued_semantic_value(), semantic_value_qualifiedpackage_or_type);
                         if (is_pointer_type == 1)
                            printf("var: type (qualified ptr) '%s' from package '%s' ", get_queued_semantic_value(), semantic_value_qualifiedpackage_or_type);
                          token = gpplex();
                     }else{
                         gpperror(1,"expected qualified type after '.'\n");
                     }
                 }else{

                    printf("var: type '%s' ", semantic_value_qualifiedpackage_or_type);
                    if(is_pointer_type == 1)
                        printf("var: type (ptr) '%s' ", semantic_value_qualifiedpackage_or_type);
                 }

             }
            } // end is_var_decl == 1
             if (token == tk_COMMA){
                token = gpplex();
                from_scope = 1;
                parse_var_decl_name_list();
                 parse_var_decl_expr();
                // HERE ??? 
               
                from_scope = 0; // or before parse_var_decl_expr(); ??? line up
            } 
        //}
        
    }

}

void parse_var_decl_expr(){
    switch(token){
        case tk_IDENT:
        case tk_STRINGLIT:
        case tk_NUM:
            printf("var: value '%s'", get_queued_semantic_value());
            token = gpplex();
            if (token == tk_COMMA){
                token = gpplex();
                // from_scope =1 ??
                parse_var_decl_expr_list();
                // from_scope = 0;
            }
    }
}

void parse_var_decl_expr_list(){
    do{
        parse_var_decl_expr();
    }while(token == tk_IDENT || token == tk_STRINGLIT || token == tk_NUM || token == tk_COMMA);
}

void parse_var_decl_name_list(){
    do{
        parse_var_decl_name();
    }while(token == tk_IDENT || token == tk_COMMA);
   // is_var_name_list_end = 1;
}

void parse_top_vars_or_const_decl(){
    parse_var_decl_name();

    if (token == tk_ASSIGN){
        token = gpplex();
        parse_var_decl_expr();
        parse_grammar();
        }else{
            if (is_var_decl == 0){ // if const declaration was/is
                gpperror(0,"const declaration cannot have type without expression");
                gpperror(1, "missing value in const declaration");

                
               // gpperror("var declaration cannot have type without expression\n");
            }

            
        }
}
/* VAR RULE END */

/*SPECIAL RULE BUILTIN TYPE BEGIN*/
void parse_builtin_type(){
    switch(token){
        case tk_T_BOOL:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'bool'");    
            printf("var: type (builtin) 'bool'");
            token = gpplex();
            break;
        case tk_T_BYTE:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'byte'");
            printf("var: type (builtin) 'byte'");
            token = gpplex();
            break;
        case tk_T_COMPLEX128:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'complex128'");
            printf("var: type (builtin) 'complex128'");
            token = gpplex();
            break;
        case tk_T_COMPLEX64:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'complex64'");
            printf("var: type (builtin) 'complex64'");
            token = gpplex();
            break;
        case tk_T_FLOAT32:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'float32'");
            printf("var type (builtin) 'float32'");
            token = gpplex();
            break;
        case tk_T_INT16:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'int16'");
            printf("var: type (builtin) 'int16'");
            token = gpplex();
            break;
        case tk_T_INT32:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'int32'");
            printf("var: type (builtin) 'int32'");
            token = gpplex();
            break;
        case tk_T_INT64:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'int64'");
            printf("var: type (builtin) 'int64'");
            token = gpplex();
            break;
        case tk_T_INT8:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'int8'");
            printf("var: type (builtin) 'int8'");
            token = gpplex();
            break;
        case tk_T_INT:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'int'");
            printf("var: type (builtin) 'int'");
            token = gpplex();
            break;
        case tk_T_RUNE:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'rune'");
            printf("var: type (builtin) 'rune'");
            token = gpplex();
            break;
        case tk_T_STRING:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'string'");
            printf("var: type (builtin) 'string'");
            token = gpplex();
            break;
        case tk_T_UINT16:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint16'");
            printf("var: type (builtin) 'uint16'");
            token = gpplex();
            break;
        case tk_T_UINT32:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint32'");
            printf("var: type (builtin) 'uint32'");
            token = gpplex();
            break;
        case tk_T_UINT64:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint64'");
            printf("var: type (builtin) 'uint64'");
            token = gpplex();
            break;
        case tk_T_UINT8:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint8'");
            printf("var: type (builtin) 'uint8'");
            token = gpplex();
            break;
        case tk_T_UINT:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint'");
            printf("var: type (builtin) 'uint'");
            token = gpplex();
            break;
        case tk_T_UINTPTR:
            if(is_pointer_type == 1)
                printf("var: type (builtin ptr) 'uintptr'");
            printf("var: type (builtin) 'uintptr'");
            token = gpplex();
            break;
    } 
}

int is_builtin_type(token_type tok)
{
    switch(tok){
        case tk_T_BOOL:
            return 1;
        case tk_T_BYTE:
            return 1;
        case tk_T_COMPLEX128:
            return 1;
        case tk_T_COMPLEX64:
            return 1;
        case tk_T_FLOAT32:
            return 1;
        case tk_T_FLOAT64:
            return 1;
        case tk_T_INT16:
            return 1;
        case tk_T_INT32:
            return 1;
        case tk_T_INT64:
            return 1;
        case tk_T_INT8:
            return 1;
        case tk_T_INT:
            return 1;
        case tk_T_RUNE:
            return 1;
        case tk_T_STRING:
            return 1;
        case tk_T_UINT16:
            return 1;
        case tk_T_UINT32:
            return 1;
        case tk_T_UINT64:
            return 1;
        case tk_T_UINT8:
            return 1;
        case tk_T_UINT:
            return 1;
        case tk_T_UINTPTR:
            return 1;
        default:
            return 0;
    }
}
/*SPECIAL RULE BUILTIN TYPE END*/
char* decl_type;
void parse_grammar()
{
    switch(token){
        case tk_PACKAGE:
            parse_top_package_stmt();
            parse_grammar();
            break;
        case tk_IMPORT:
        case tk_GOIMPORT:
            parse_top_import_decl();
            parse_grammar();
            break;
        /* TEST RULE; BEGIN THIS */
        // case tk_CLASS:
        //     printf("hello class\n");
        //     token = gpplex();
        //     parse_grammar();
        //     break;
        case tk_VAR:
            is_var_decl = 1;
            parse_top_vars_or_const_decl();
            parse_grammar();
            break;
        case tk_CONST:
            is_var_decl = 0;
            parse_top_vars_or_const_decl();
            parse_grammar();
            break;
        case tk_IDENT:
        case tk_NUM:
        case tk_STRINGLIT:
            gpperror(1,"unexpected '%s', expected declaration or statement", get_queued_semantic_value());
    }
}


void gppparse()
{
    token = gpplex();
    if(token != tk_PACKAGE)
        gpperror(1,"expected package statement\n");
    do{
        parse_grammar();
    }while(token != tk_EOF);
}