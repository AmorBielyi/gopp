#include <stdio.h> // for printf
#include <string.h> // for _strdup
#include <stdlib.h>
#include "../lexer/lexer.h"

/*
    globals
*/
static token_type _token;
char *_semantic_value;
extern char *filename;
/*
    prototypes
*/
void next_token(void);
void syntax_error(char *msg);
void error(char *msg);

/*
  definitions
*/

void next_token(void)
{
    _semantic_value = _strdup(get_queued_semantic_value());
    _token = apxlex();
    

}

void error(char *msg)
{
    //next_token();
    printf("\n%s: error: %s", filename, msg); exit(1);
}

void syntax_error(char *msg)
{
    printf("\n%s: expected %s, found '%s'", filename, msg, _semantic_value );exit(1);
}


int lookahead(token_type token)
{
    if (_token == token){
        next_token();
        return 1;
    }
    return 0;
}

int accept(token_type token)
{
    if (_token == token)
    {
       // _semantic_value = _strdup(get_queued_semantic_value());
        next_token();
        return 1;
    }

    return 0;
}

int expect(token_type token, char *what_expected)
{
    if (accept(token))
        return 1;
    syntax_error(what_expected);
}

token_type get_builtin_type(token_type token)
{
 switch(token)
 {
    case tk_T_BOOL:
        return tk_T_BOOL;
    case tk_T_BYTE:
        return tk_T_BYTE;
    case tk_T_COMPLEX128:
        return tk_T_COMPLEX64;
    case tk_T_COMPLEX64:
        return tk_T_COMPLEX64;
    case tk_T_FLOAT32:
        return tk_T_FLOAT32;
    case tk_T_FLOAT64:
        return tk_T_FLOAT64;
    case tk_T_INT16:
        return tk_T_INT16;
    case tk_T_INT32:
        return tk_T_INT32;
    case tk_T_INT64:
        return tk_T_INT64;
    case tk_T_INT8:
        return tk_T_INT8;
    case tk_T_INT:
        return tk_T_INT;
    case tk_T_RUNE:
        return tk_T_RUNE;
    case tk_T_STRING:
        return tk_T_STRING;
    case tk_T_UINT16:
        return tk_T_UINT16;
    case tk_T_UINT32:
        return tk_T_UINT32;
    case tk_T_UINT64:
        return tk_T_UINT64;
    case tk_T_UINT8:
        return tk_T_UINT8;
    case tk_T_UINT:
        return tk_T_UINT;
    case tk_T_UINTPTR:
        return tk_T_UINTPTR;
 } 
 return -1;
}

/*
    ACTIONS
*/
void action_package(void)
{
    printf("package name: '%s'\n", _semantic_value);
}

int include_type = 0;
enum include_types {
    INCLUDE_TYPE_INCLUDE = 1,
    INCLUDE_TYPE_GOINCLUDE
};
void action_include_body_do_source(void)
{
    if (include_type == INCLUDE_TYPE_INCLUDE)
        printf("include: source '%s'", _semantic_value);
    if (include_type == INCLUDE_TYPE_GOINCLUDE)
        printf("goinclude: source '%s'", _semantic_value);
}

void action_include_body_do_alias(void)
{
    printf(" alias: '%s'", _semantic_value);
}

int statement_type = 0;
enum statement_types {
    STATEMENT_TYPE_VAR = 1,
    STATEMENT_TYPE_CONST
};
void action_pointer(void)
{
    if (statement_type == STATEMENT_TYPE_VAR)
        printf("var: pointer = true");
    if(statement_type == STATEMENT_TYPE_CONST)
        printf("const: pointer = true");
}

void action_user_type(void)
{
    if (statement_type == STATEMENT_TYPE_VAR)
        printf("\nvar: usertype: '%s'\n", _semantic_value);
    if (statement_type == STATEMENT_TYPE_CONST)
        printf("\nconst: usertype: '%s'\n", _semantic_value);
}

void action_user_type_do_qualified(char *package_name)
{
    if (statement_type == STATEMENT_TYPE_VAR)
        printf("\nvar: usertype (qualified) '%s', package '%s'\n", _semantic_value, package_name);
    if (statement_type == STATEMENT_TYPE_CONST)
        printf("\nconst: usertype (qualified) '%s', package '%s'\n", _semantic_value, package_name);
}

void action_ident_list(void)
{
    printf("ident '%s'", _semantic_value);
}

void action_value_list(void)
{
    printf(" value '%s'", _semantic_value);
}

void action_var_do_builtin_type(void)
{
    if (statement_type == STATEMENT_TYPE_VAR)
        printf("\nvar: type (builtin) '%s' ", _semantic_value);
    if (statement_type == STATEMENT_TYPE_CONST)
        printf("\nconst: type (builtin) '%s' ", _semantic_value);
}

/*
    RULES
*/

void terminator(void)
{
    expect(tk_SEMI, "';'");
}



void value_list(void)
{
    do 
    {
        //printf("\ntoken: '%i'", _token);
        if (expect(tk_NUM, "value"))
            action_value_list();
    }while(accept(tk_COMMA));
}

void ident_list(void)
{
    do
    {
        if(expect(tk_IDENT, "identifier"));
            action_ident_list();
    }while(accept(tk_COMMA));
}

void qualified_user_type(void)
{
    char *backed_semantic = _strdup(_semantic_value);
            expect(tk_IDENT, "qualified user type");
            action_user_type_do_qualified(backed_semantic);
    return;
}

void user_type(void )
{
   if (accept(tk_IDENT)){
        if (lookahead(tk_DOT))
            qualified_user_type();
        else 
            action_user_type();   
   }      
}

void builtin_type(void)
{
    action_var_do_builtin_type();
}

void type(void)
{
    if (lookahead(get_builtin_type(_token)))
        builtin_type();
    else 
        user_type();
    if (lookahead(tk_MUL))
    {
        action_pointer();
    }
    
        
}
void init(void)
{
    type();
    ident_list();
    if(lookahead(tk_ASSIGN))
        value_list(); 
}
void var(void)
{
    init();
}

void constt(void)
{
       init();
}

void include_body(void)
{
    expect(tk_STRINGLIT, "source");
        action_include_body_do_source();
            
        if (lookahead(tk_AS))
        {
            expect(tk_IDENT, "alias");
            action_include_body_do_alias();
        }
        printf("\n");
        terminator();
}

void statement(void)
{
    
    if (accept(tk_VAR))
    {
        statement_type = STATEMENT_TYPE_VAR;
        var();
        terminator();
        statement();
    }
    if (accept(tk_CONST))
    {
        statement_type = STATEMENT_TYPE_CONST;
        constt();
        terminator();
        statement();
    }
}

void statement_list(void)
{
    statement();
}

void includes_list()
{
    if (accept(tk_INCLUDE))
    {
        include_type = INCLUDE_TYPE_INCLUDE;
        include_body();
        includes_list();  
    }
    if (accept(tk_GOINCLUDE))
    {
        include_type = INCLUDE_TYPE_GOINCLUDE;
        include_body();
        includes_list();
    }
    
}

void package(void)
{
    if (accept(tk_PACKAGE))
        expect(tk_IDENT, "package name");
    else
        error("missed package");
    action_package();
    terminator();
}

void source(void)
{
    next_token();
    package();
    includes_list();
    statement_list();
}

void apxparse()
{
    source();
    if (lookahead(tk_IDENT) || lookahead(tk_NUM) || lookahead(tk_STRINGLIT))
        syntax_error("statement");
}