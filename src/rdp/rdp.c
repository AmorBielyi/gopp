#include <stdio.h> // for printf
#include <string.h> // for _strdup
#include <stdlib.h>
#include "../lexer/lexer.h"

/*
    globals
*/
static token_type _token;
char *_semantic_value;
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
    _token = apxlex();
   // _semantic_value = get_queued_semantic_value();

}

void error(char *msg)
{
    //next_token();
    printf("error: %s", msg); exit(1);
}

void syntax_error(char *msg)
{
    printf("expected '%s', found '%s'", msg, _semantic_value );exit(1);
}

int accept(token_type token)
{
    if (_token == token)
    {
        _semantic_value = _strdup(get_queued_semantic_value());
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

void include_body(void)
{
    expect(tk_STRINGLIT, "STRING LITERAL");
        printf("include: source '%s'", _semantic_value);
            
        if (_token == tk_AS)
        {
            next_token();
            expect(tk_IDENT, "ALIAS");
            printf(" alias: '%s'", _semantic_value);
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
        expect(tk_IDENT, "PACKAGE NAME");
    else
        error("missed package");
    printf("package name: '%s'\n", _semantic_value);
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
}