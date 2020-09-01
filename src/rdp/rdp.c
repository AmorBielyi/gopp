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

/*
    ACTIONS
*/
void action_package(void)
{
    printf("package name: '%s'\n", _semantic_value);
}

void action_include_body_do_source(void)
{
    printf("include: source '%s'", _semantic_value);
}

void action_include_body_do_alias(void)
{
    printf(" alias: '%s'", _semantic_value);
}

/*
    RULES
*/
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
}

void includes_list()
{
    if (accept(tk_INCLUDE))
    {
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
}

void source(void)
{
    next_token();
    package();
    includes_list();
}

void apxparse()
{
    source();
    if (lookahead(tk_IDENT) || lookahead(tk_NUM) || lookahead(tk_STRINGLIT))
        syntax_error("statement");
}