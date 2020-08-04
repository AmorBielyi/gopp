#include <stdio.h>
#include <string.h>
#include "../lexer/lexer.h"

token_type token;


void parse_grammar()
{
    switch(token){
        case tk_PACKAGE:
            token = gpplex();
            if(token = tk_IDENT)
                printf("package name: %s", get_queued_semantic_value());
            else
                gpperror("expected package name");
    }
}

void gppparse()
{
    token = gpplex();
    do{
        parse_grammar();
    }while(token != tk_EOF);
}