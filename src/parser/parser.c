#include <stdio.h>
#include <string.h>
#include "../lexer/lexer.h"

token_type token;
void parse_grammar();
void parse_import_decl_without_alias();
void parse_import_decl_with_alias();
void parse_imports_decl_scope();

/* PACKAGE RULE BEGIN */
void parse_package_stmt()
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
void parse_imports_decl()
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
            do {
                parse_imports_decl_scope();
            }while(token == tk_IDENT || token == tk_STRINGLIT) ;
            from_scope = 0;
            if (token == tk_RPAREN){
                 token = gpplex();
                 break;
             }else{
                 gpperror("expected ')' after import scope\n");
             }
    }
        
}

void parse_imports_decl_scope(){
    
    parse_imports_decl();
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


void parse_grammar()
{
    switch(token){
        case tk_PACKAGE:
            parse_package_stmt();
            parse_grammar();
            break;
        case tk_IMPORT:
        case tk_GOIMPORT:
            parse_imports_decl();
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