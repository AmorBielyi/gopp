#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys\timeb.h> 
//#include "lexer/lexer.h"
#include "parser/parser.h"

extern void create_dump();
long TOKENSDUMP;

FILE *source_fp;
FILE* tokens_stream_dump;
char * filename;
int main(int argc, char *argv[])
{
    
    if (argc >= 2){
        TOKENSDUMP  = strtol(argv[2], NULL, 10);
        if (TOKENSDUMP == 1)
            create_dump();
        filename = argv[1];
        char *ssc;
        int l = 0;
        ssc = strstr(filename, "\\");
        do{
            l = strlen(ssc) + 1;
            filename = &filename[strlen(filename)-l+2];
            ssc = strstr(filename, "\\");
        }while(ssc);
        //printf("filename: %s\n", filename);
        errno_t err = fopen_s(&source_fp, argv[1], "r");
        if (err == 0){
            printf("\nBegin parsing %s\n\n", argv[1]);
           // lex();
            struct timeb start, end;
            ftime(&start);

            gppparse(); // call bison that call internally our yylex() by extern

            fclose(source_fp);
            if (TOKENSDUMP == 1)
                fclose(tokens_stream_dump);
            ftime(&end);
            double diff = (double)(1000.0 * (end.time - start.time) + (end.millitm - start.millitm));
            printf("\nSuccessfully parsed\nTime: %f seconds = %f milliseconds = %f microseconds", diff / 1000, diff, diff * (double)1000);
        }else{
          fprintf(stderr, "Error: can't open source file %s ", argv[1]); exit(1);
        }
    }else{
        fprintf(stderr, "Error, source file not specified"); exit(1);
    }

    return 0;
}


// ----^ save line; upper all is valid !!!!!!!!!!!!!!!!

