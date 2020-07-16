#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lexer/lexer.h"


FILE *source_fp;

int main(int argc, char *argv[])
{
  
    if (argc >= 2){
        TOKENSDUMP  = strtol(argv[2], NULL, 10);
        errno_t err = fopen_s(&source_fp, argv[1], "r");
        if (err == 0){
            printf("\nBegin lexing %s\n\n", argv[1]);
            lex();
        }else{
          fprintf(stderr, "Error: can't open source file %s ", argv[1]); exit(1);
        }
    }else{
        fprintf(stderr, "Error, source file not specified"); exit(1);
    }

    return 0;
}