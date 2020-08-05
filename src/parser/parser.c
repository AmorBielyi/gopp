#include <stdio.h>
#include <string.h>
#include "../lexer/lexer.h"

token_type token;
void parse_grammar();
void parse_import_decl_without_alias();
void parse_import_decl_with_alias();
void parse_recc_imports_decl_scope();
void parse_var_decl_name_list();
void parse_var_decl_expr_list();
void parse_var_decl_expr();

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
        gpperror("expected package name");
}
/* PACKAGE RULE END */

int from_scope = 0;
/* IMPORTS RULE BEGIN */
void parse_top_imports_decl()
{
    if (from_scope != 1){
        token = gpplex();
    }
    
    switch(token){
        case tk_STRINGLIT:
            parse_import_decl_without_alias(get_queued_semantic_value());
           break;
        case !tk_STRINGLIT:
            gpperror("expected import source");
        case tk_IDENT:
            parse_import_decl_with_alias(get_queued_semantic_value());
            break;
        case tk_LPAREN:
            token = gpplex();
            from_scope = 1;
            parse_recc_imports_decl_scope();
            from_scope = 0;
            if (token == tk_RPAREN){
                 token = gpplex();
                 break;
             }else{
                 gpperror("expected ')' after import scope\n");
             }
    }
        
}

void parse_recc_imports_decl_scope(){
    do {
         parse_top_imports_decl();
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
        gpperror("expected package source after alias");
        
}
/* IMPORTS RULE END */
int is_var_name_list_end = 0;

/* VAR RULE BEGIN */
void parse_var_decl_name(){
    if (from_scope != 1) {
        token = gpplex();
    }
    switch(token){
        if (is_var_name_list_end == 0){
            case tk_IDENT:
             printf("var: name '%s' ", get_queued_semantic_value());
             token = gpplex();
             if(token == tk_IDENT){
                 printf("var: type '%s' ", get_queued_semantic_value());
                 token = gpplex();
             }
             if (token == tk_COMMA){
                token = gpplex();
                from_scope = 1;
                parse_var_decl_name_list();
                 parse_var_decl_expr();
                // HERE ??? 
               
                from_scope = 0; // or before parse_var_decl_expr(); ??? line up
            } 
        }
        
    }

}

void parse_var_decl_expr(){
    switch(token){
        case tk_IDENT:
        case tk_STRINGLIT:
        case tk_NUM:
            printf("var: value '%s, '", get_queued_semantic_value());
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
    is_var_name_list_end = 1;
}

void parse_top_vars_decl(){
    parse_var_decl_name();
   // printf("tok: %u", token);
    // if (token == tk_IDENT){
    //     printf("var: type '%s' ", get_queued_semantic_value());
    //     token = gpplex();
    // }
  //  printf("tok: %u", token);
    if (token == tk_ASSIGN){
        token = gpplex();
        parse_var_decl_expr();
        parse_grammar();
        }else{
            gpperror("uninitialized variable, expected assigned value\n");
        }
}
/* VAR RULE END */



void parse_grammar()
{
    switch(token){
        case tk_PACKAGE:
            parse_top_package_stmt();
            parse_grammar();
            break;
        case tk_IMPORT:
        case tk_GOIMPORT:
            parse_top_imports_decl();
            parse_grammar();
            break;
        /* TEST RULE; BEGIN THIS */
        // case tk_CLASS:
        //     printf("hello class\n");
        //     token = gpplex();
        //     parse_grammar();
        //     break;
        case tk_VAR:
            parse_top_vars_decl();
            parse_grammar();
            break;
        case tk_IDENT:
        case tk_NUM:
        case tk_STRINGLIT:
            gpperror("unexpected '%s', expected declaration or statement", get_queued_semantic_value());
    }
}


void gppparse()
{
    token = gpplex();
    if(token != tk_PACKAGE)
        gpperror("expected package statement\n");
    do{
        parse_grammar();
    }while(token != tk_EOF);
}