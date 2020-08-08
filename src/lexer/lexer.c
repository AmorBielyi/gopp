#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <sys\timeb.h> 
//#include "..\bisonparser\parser.h"
#define NELEMS(arr) (sizeof(arr) / sizeof(arr[0]))

#define da_dim(name, type)  type *name = NULL;          \
                            int _qy_ ## name ## _p = 0;  \
                            int _qy_ ## name ## _max = 0
#define da_rewind(name)     _qy_ ## name ## _p = 0
#define da_redim(name)      do {if (_qy_ ## name ## _p >= _qy_ ## name ## _max) \
                                name = realloc(name, (_qy_ ## name ## _max += 56) * sizeof(name[0]));} while (0)
#define da_append(name, x)  do {da_redim(name); name[_qy_ ## name ## _p++] = x;} while (0)
#define da_len(name)        _qy_ ## name ## _p

  typedef enum 
  {
    tk_EOF, 
    tk_BREAK,
    tk_CASE ,
    tk_CHAN ,
    tk_CONST,
    tk_CONTINUE,
    tk_DEFAULT,
    tk_DEFER,
    tk_ELSE ,
    tk_FALLTHROUGH,
    tk_FOR,
    tk_GO,
    tk_GOTO,
    tk_IF,
    tk_INTERFACE,
    tk_FUNC,
    tk_MAP,
    tk_PACKAGE,
    tk_RANGE,
    tk_RETURN,
    tk_SELECT,
    tk_STRUCT,
    tk_SWITCH,
    tk_TYPE,
    tk_VAR,
    tk_CLASS,
    tk_EXTENDS,
    tk_IMPLEMENTS,
    tk_THIS,
    tk_NEW,
    tk_SUPER,
    tk_PUBLIC,
    tk_PRIVATE,
    tk_PTRSELECT,
    tk_OVERRIDE,
    tk_IMPORT,
    tk_GOIMPORT,
    tk_T_STRING,
    tk_T_BOOL,
    tk_T_INT8,
    tk_T_UINT8,
    tk_T_BYTE,
    tk_T_INT16,
    tk_T_UINT16,
    tk_T_INT32,
    tk_T_UINT32,
    tk_T_RUNE,
    tk_T_INT64,
    tk_T_UINT64,
    tk_T_INT,
    tk_T_UINT,
    tk_T_UINTPTR,
    tk_T_FLOAT32,
    tk_T_FLOAT64,
    tk_T_COMPLEX64,
    tk_T_COMPLEX128,
    tk_ADD,
    tk_SUB,
    tk_MUL,
    tk_DIV,
    tk_MOD,
    tk_AND,
    tk_OR,
    tk_XOR ,
    tk_ASSIGN,
    tk_LPAREN,
    tk_RPAREN,
    tk_LSBRACKET,
    tk_RSBRACKET,
    tk_LCBRACKET,
    tk_RCBRACKET,
    tk_COMMA,
    tk_DOT,
    tk_SEMI,
    tk_UNDERSCORE,
    tk_COLON,
    tk_LSHIFT,
    tk_RSHIFT,
    tk_EQXOR,
    tk_EQOR,
    tk_EQAND,
    tk_EQANDXOR,
    tk_EQRSHIFT,
    tk_EQLSHIFT,
    tk_LOGICAND,
    tk_LOGICOR,
    tk_EQADD,
    tk_EQSUB,
    tk_EQMUL,
    tk_EQDIV,
    tk_EQMOD,
    tk_ANDXOR,
    tk_NEG,
    tk_LSS,
    tk_GRT,
    tk_NOTEQ,
    tk_EQ,
    tk_EQLSS,
    tk_EQGRT,
    tk_SHORTDECL,
    tk_ARROW,
    tk_INC,
    tk_DEC,
    tk_ELLIPSIS,
    tk_STRINGLIT,
    tk_NUM,
    tk_IDENT,
    tk_TRUE,
    tk_FALSE,
    tk_IOTA
  } token_type;


extern long TOKENSDUMP;
extern FILE* tokens_stream_dump;
//typedef int token_type;
int gpplex();
void write_dump(char* token_name, char* token_value);
void dump_keywords(token_type token);

char *reserved_semantic_value = NULL;
char *semantic_value = NULL;

int lex_semantic_value_queue_state = -1 ;

char* get_queued_semantic_value();

struct symbol_table_entry{
    char *semantic_value;
};

#define NHASH 9997

struct symbol_table_entry symbol_table[NHASH];

int insert_symbol_table(char * semantic_value);

unsigned hash_symbol_table(char *semantic_value);


extern FILE *source_fp;

int line =1, col = 0, the_ch = ' ';
da_dim(text, char);

void gpperror(const char *fmt, ...)
{
    char buf[1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf_s(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("\n%d:%d: %s", line, col, buf);
    exit(1);
}

char* get_queued_semantic_value() {
    if ( reserved_semantic_value != NULL)
        return reserved_semantic_value;
   
    return semantic_value;
         
}


static int next_ch()
{  /* get next char from our input */
    the_ch = getc(source_fp);
    ++col;
    if(the_ch == '\n'){
        ++line;
        col = 0;
    }
    return the_ch;
}


static int back_ch(int ch)
{
    ungetc(ch , source_fp);
}

static token_type char_lit(int n)
{ /* 'x' */
 if (the_ch == '\'')
    gpperror("Syntax error: empty character constant");
if (the_ch == '\\'){
    next_ch();
    if(the_ch == 'n')
        n = 10;
    else if (the_ch == '\\')
        n = '\\';
    else gpperror( "Syntax error: unknown escape sequence \\%c", the_ch);
} 
if(next_ch() != '\'')
    gpperror("Syntax error: multi-character constant");
next_ch();
semantic_value = (char *)n; // maybe error here 

if(TOKENSDUMP == 1)
    fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_NUM\t\tSemanticValue: '%hs'\n",line, col, semantic_value);

return tk_NUM;

}

static token_type div_or_cmt() {  /* (divide)'/' or comment */
    if (the_ch != '*')

        if (TOKENSDUMP == 1)
             write_dump("tk_DIV", "/");

        return tk_DIV;
       
    if (the_ch == '/')
        printf("\nsingle comment\n");
   /* when comment found */
   next_ch();
    for (;;) {
        
        if (the_ch == '*') {
            if (next_ch() == '/') {
                next_ch();
                return gpplex();
            }
        } else if (the_ch == EOF)
            gpperror("EOF in comment");
        else
            next_ch();
    }
}


static token_type string_lit(int start)
{ 
    /* "string literal" */
    da_rewind(text);

    while(next_ch() != start){
        if(the_ch == '\n') gpperror( "Syntax error: EOL in string");
        if (the_ch == EOF) gpperror( "Syntax error: EOF in string");
        da_append(text, (char)the_ch);
    }
    da_append(text, '\0');

    next_ch();
    if (lex_semantic_value_queue_state == -1) {
        reserved_semantic_value = text;
        lex_semantic_value_queue_state++;
        
        return tk_STRINGLIT;
    }
    if (lex_semantic_value_queue_state > -1) {
        lex_semantic_value_queue_state--;
        semantic_value = text;
        if (TOKENSDUMP == 1)
            fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_STRINGLIT\t\tSemanticValue: '%hs'\n", line, col, semantic_value);
        return tk_STRINGLIT;
    }
    
    semantic_value = text;
    return tk_STRINGLIT;
}

static int kwd_cmp(const void *p1, const void *p2){
    return strcmp(*(char **)p1, *(char **)p2);
}



 unsigned hash_symbol_table(char *semantic_value)
{
    unsigned int hash = 0;
    unsigned c;

    while(c = *semantic_value++) hash = hash*9 ^c;
    return hash; 
}

int insert_symbol_table(char *semantic_value)
{
    struct symbol_table_entry *entry = &symbol_table[hash_symbol_table(semantic_value) % NHASH];
    int scount = NHASH;

    while(--scount >=0){
        
        if(!entry->semantic_value){
            entry->semantic_value = _strdup(semantic_value);
            return 1;
        }
        if(++entry >= symbol_table +NHASH) entry = symbol_table;
    }
    gpperror("symbol table overflow\n");
    abort();
}

int lookup_symbol_table(char* semantic_value)
{
   struct symbol_table_entry *entry = &symbol_table[hash_symbol_table(semantic_value) % NHASH];
   int scount = NHASH;

   while(--scount >= 0){
       if(entry->semantic_value && !strcmp(entry->semantic_value, semantic_value))
       {
           return 1;
       }
       if(++entry >= symbol_table +NHASH) entry = symbol_table;
   }
   //printf("symbol not found in symbol table: '%s' \n", get_queued_semantic_value());
   return 0;
}

static token_type get_keyword_type(const char *ident)
{
    static struct{
        char *s;
        token_type sym;
    } kwds[] = {
       
        /* Original keywords */
        {"break", tk_BREAK},
        {"case", tk_CASE},
        {"chan", tk_CHAN},
        {"const", tk_CONST},
        {"continue", tk_CONTINUE},
        {"default", tk_DEFAULT},
        {"defer", tk_DEFER},
        {"else", tk_ELSE},
        {"fallthrough", tk_FALLTHROUGH},
        {"for", tk_FOR},
        {"goto", tk_GOTO},
        {"go", tk_GO},
        {"if", tk_IF},
        {"interface", tk_INTERFACE},
        {"map", tk_MAP},
        {"package", tk_PACKAGE},
        {"range", tk_RANGE},
        {"return", tk_RETURN},
        {"select", tk_SELECT},
        {"struct", tk_STRUCT},
        {"switch", tk_SWITCH},
        {"type", tk_TYPE},
        {"var", tk_VAR},
        /* GoToClass' original keywords */
        {"class", tk_CLASS},
        {"extends", tk_EXTENDS},
        {"implements", tk_IMPLEMENTS},
        {"this", tk_THIS},
        {"new", tk_NEW},
        {"super", tk_SUPER},
        {"public", tk_PUBLIC},
        {"private", tk_PRIVATE},
        {"override", tk_OVERRIDE},
        {"import", tk_IMPORT},
        {"goimport", tk_GOIMPORT},
        /* Original built-in types */ 
        {"string", tk_T_STRING},
        {"bool", tk_T_BOOL},
        {"int8", tk_T_INT8},
        {"uint8", tk_T_UINT8},
        {"byte", tk_T_BYTE},
        {"int16", tk_T_INT16},
        {"uint16", tk_T_UINT16},
        {"int32", tk_T_INT32},
        {"uint32", tk_T_UINT32},
        {"rune", tk_T_RUNE},
        {"int64", tk_T_INT64},
        {"uint64", tk_T_UINT64},
        {"int", tk_T_INT},
        {"uint", tk_T_UINT},
        {"uintptr", tk_T_UINTPTR},
        {"float32", tk_T_FLOAT32},
        {"float64", tk_T_FLOAT64},
        {"complex64", tk_T_COMPLEX64},
        {"complex128", tk_T_COMPLEX128},
        /*Original misc*/
        {"true", tk_TRUE},
        {"false", tk_FALSE},
        {"iota", tk_IOTA},

    },*kwp; 
    qsort(kwds, NELEMS(kwds), sizeof(kwds[0]), kwd_cmp);
    kwp = bsearch(&ident, kwds, NELEMS(kwds), sizeof(kwds[0]), kwd_cmp);
    if (kwp == NULL) {
        // if (TOKENSDUMP == 1)
        //     fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_IDENT\t\tSemanticValue: '%hs'\n", line, col, get_queued_semantic_value());
            
        if (lex_semantic_value_queue_state == -1) {
            reserved_semantic_value = text;
            lex_semantic_value_queue_state++;
            if (TOKENSDUMP == 1)
            fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_IDENT\t\tSemanticValue: '%hs'\n", line, col, get_queued_semantic_value());
            return tk_IDENT;
        }
        if (lex_semantic_value_queue_state > -1) {
            lex_semantic_value_queue_state--;
            semantic_value = text;
            if (TOKENSDUMP == 1)
            fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_IDENT\t\tSemanticValue: '%hs'\n", line, col, get_queued_semantic_value());
            return tk_IDENT;
        }
    }
    
    if (TOKENSDUMP == 1) {
        token_type token = kwp->sym;
        dump_keywords(token);
    }

    return kwp->sym;
}

int isutf8unicode(int c)
{
    return !isascii(c); 
}

static token_type ident_or_num()
{   int n = 0;
    int is_ident = false;
    da_rewind(text);
   
    while(isalnum(the_ch) || the_ch == '_' || isutf8unicode(the_ch) ){
        
        da_append(text, the_ch);
        if (!isdigit(the_ch))
            is_ident = true;
        next_ch();
    }
    
    if (da_len(text) == 0)
        gpperror("Syntax error: Syntax error: unrecognized character, code:  (%d) '%c'\n", the_ch, the_ch);

  
    
    da_append(text, '\0');
  
    /* only for chars that >= 1 (non-unicode chars) to avoid debug assetion failed error in isdigit() 
    and if first char of ident is digit return syntax error according to Go rule  */
        if(text[0] >=1 && isdigit(text[0])){
            if(is_ident)
                gpperror("Syntax error: invalid identifier, can't begin with digit '%c'", text[0], text[0]);
             n = strtol(text, NULL, 0);
            if (n == LONG_MAX && errno == ERANGE)
                gpperror("Syntax error: Number exceeds maximum value");
            semantic_value = text;
            if (TOKENSDUMP == 1)
                fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_NUM\t\tSemanticValue: '%hs'\n", line, col, semantic_value);
            return tk_NUM;
            
        } 

        return get_keyword_type(text);
    
}


static token_type LA
(
    int except, 
    token_type foundnext, 
    token_type self
    )
{
  
    if (the_ch == except){
         next_ch();
         return foundnext;
       
    }
    if(self == EOF)
        gpperror("Syntax error (LA): Syntax error: unrecognized character, code:  '%c' (%d)\n", the_ch, the_ch);
    return self;

}

static token_type LA2
(
    int except1, 
    int except2, 
    token_type foundl2, 
    token_type foundl1, 
    token_type self
)
{
    if (self == EOF)
            gpperror( "Syntax error (LA n2): Syntax error: unrecognized character, code:  '%c' (%d)\n", the_ch, the_ch);

    if (the_ch == except1){
        next_ch();
    
        if (the_ch == except2){
            next_ch();
            return foundl2;
        }else{
            if (foundl1 != 0)
                return foundl1;
              
        }
    }
          return self;
  
}


int gpplex()
{
    
    /* skip whitespace */
    while(isspace(the_ch))
        next_ch();
    switch(the_ch){
        case '+':
        {
            next_ch();
            token_type token = LA('+', tk_INC, tk_ADD);
            if (token == tk_INC)
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_INC", "++");
                return token;
            }
               
            token = LA('=', tk_EQADD, tk_ADD);
            if (token == tk_EQADD)
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQADD", "+=");
                return token;
            }
                
            if (token == tk_ADD) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_ADD", "+");
                return token;
            }
                 
        }
        case '-':
        {
            next_ch();
            token_type token = LA('>', tk_PTRSELECT, tk_SUB);
            if (token == tk_PTRSELECT) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_PTRSELECT", "->");
                return token;
            }
                
            token = LA('-', tk_DEC, tk_SUB);
            if (token == tk_DEC) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_DEC", "--");
                return token;
            }
                
            token = LA('=', tk_EQSUB, tk_SUB);
            if (token == tk_EQSUB) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQSUB", "-=");
                return token;
            }
                
            if (TOKENSDUMP == 1)
                write_dump("tk_SUB", "-");
            return token; 
        }
        case '*':
        {
            next_ch(); 
            token_type token = LA('=', tk_EQMUL, tk_MUL);
            if (token == tk_EQMUL) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQMUL", "*=");
                return token;
            }
            if (TOKENSDUMP == 1)
                write_dump("tk_MUL", "*");
            return token;
                
        }
        case '/':
        {
          next_ch();
          /* skip single comment before '\n' */
          if (the_ch == '/'){ 
              do{
                  next_ch();
              }while(the_ch != '\n');
              next_ch(); return gpplex();
          }

          token_type token = LA('=', tk_EQDIV, tk_DIV);
          if (token == tk_EQDIV) 
          {   
              if (TOKENSDUMP == 1)
                  write_dump("tk_EQDIV", "/=");
              return token;
          }
                
          /* skip multi comment before */
          return div_or_cmt(); 
        }

        case '%': next_ch(); return LA('=', tk_EQMOD, tk_MOD);
        case '&': 
        {   
            next_ch(); 
            token_type token = LA('=', tk_EQAND, tk_AND);
            if (token == tk_EQAND)
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQAND", "&=");
                return token;
            }
                
            token = LA('&', tk_LOGICAND, tk_AND);
            if (token == tk_LOGICAND) 
            {   
                if (TOKENSDUMP == 1)
                    write_dump("tk_LOGICAND", "&&");
                return token;
            }
                
            token = LA2('^', '=', tk_EQANDXOR, tk_ANDXOR, tk_AND);
            
            if (token == tk_EQANDXOR) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQANDXOR", "&^=");
                return token;
            }
            if (token == tk_ANDXOR)
            {
                if(TOKENSDUMP == 1)
                    write_dump("tk_ANDXOR", "&^");
                return token;
            }
            if (token == tk_AND) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_AND", "&");
                return token;
            }
            
        }
        case '|':
        {
            next_ch();
            token_type token = LA('|', tk_LOGICOR, tk_OR);
            if (token == tk_LOGICOR) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_LOGICOR", "||");
                return token;
            }
            token = LA('=', tk_EQOR, tk_OR);
            if (token == tk_EQOR) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQOR", "&=");
                return token;
            }
                
            if (token == tk_OR) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_OR", "|");
                return token;
            }
               
        }
        case '^': next_ch(); return LA('=', tk_EQXOR, tk_XOR);
        case '=': next_ch(); return LA('=', tk_EQ, tk_ASSIGN);
        case '(':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_LPAREN", "(");
            return tk_LPAREN;
        }
        case ')':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_RPAREN", ")");
            return tk_RPAREN;
        }
        case '[':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_LSBRACKET", "[");
            return tk_LSBRACKET;
        }
        case ']':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_RSBRACKET", "]");
            return tk_RSBRACKET;
        }
        case '{':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_LCBRACKET", "{");
            return tk_LCBRACKET;
        }
        case '}':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_RCBRACKET", "}");
            return tk_RCBRACKET;
        }
        case ',':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_COMMA", ",");
            return tk_COMMA;
        }
        case '.':
        {
            next_ch();
            if (the_ch == '.'){
                next_ch();
                if (the_ch == '.'){
                    next_ch();
                    if (TOKENSDUMP == 1)
                        write_dump("tk_ELLIPSIS", "...");
                    return tk_ELLIPSIS;
                }else 
                   gpperror( "Syntax error (LA): unrecognized character, code:  .\n", the_ch, the_ch);
                
            }
            if (TOKENSDUMP == 1)
                write_dump("tk_DOT", ".");
            return tk_DOT;

        } 
        case ';':
        {
            next_ch();
            if (TOKENSDUMP == 1)
                write_dump("tk_SEMI", ";");
            return tk_SEMI;
        }
        case ':': 
        {
            next_ch(); 
            token_type token =  LA('=', tk_SHORTDECL, tk_COLON);
            if (token == tk_SHORTDECL) {
                if (TOKENSDUMP == 1)
                    write_dump("tk_SHORTDECL", ":=");
                return token;
            }
            if (token == tk_COLON) {
                if (TOKENSDUMP == 1)
                    write_dump("tk_COLON", ":");
                return token;
            }
        }
        
        case '!':
        {
            next_ch(); 
            token_type token = LA('=', tk_NOTEQ, tk_NEG);
            if (token == tk_NOTEQ) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_NOTEQ", "!=");
                return token;
            }
            if (token == tk_NEG) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_NEG", "!");
                return token;
            }
        }
        case '<':
        {
            next_ch();
            token_type token = LA('=', tk_EQLSS, tk_LSS);
            if (token == tk_EQLSS)
                return token;
            token = LA('-', tk_ARROW, tk_LSS);
            if (token == tk_ARROW)
                return token;
            token = LA2('<', '=', tk_EQLSHIFT, tk_LSHIFT, tk_LSS);
            if (token == tk_EQLSHIFT || token == tk_LSHIFT)
                return token;
            if (token == tk_LSS)
                return token;

        } 
        case '>':
        {
            next_ch();
            token_type token = LA('=', tk_EQGRT, tk_GRT);
            if (token == tk_EQGRT)
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQGRT", ">=");
                return token;
            }
                
            token = LA2('>', '=', tk_RSHIFT, tk_EQRSHIFT, tk_GRT);
            if (token == tk_RSHIFT || token == tk_EQRSHIFT)
                return token;
            if (token == tk_GRT)
                return token;

            if (token == tk_RSHIFT) 
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_RSHIFT", ">>");
                return token;
            }
            if (token == tk_EQRSHIFT)
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_EQRSHIFT", ">>=");
                return token;
            }
            if (token == tk_GRT)
            {
                if (TOKENSDUMP == 1)
                    write_dump("tk_GRT", ">");
                return token;
            }
        }
        case '"': return string_lit(the_ch);
         
        // case '\'': next_ch(); return char_lit(the_ch, err_line, err_col);
        
        case EOF:
        {
            if (TOKENSDUMP == 1)
                fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_EOF\n", line, col);
            return tk_EOF;
        }

        default: 
        {
            return ident_or_num();
        }
        
        
    }
    gpperror("unknown");
}



void create_dump()
{
   errno_t res = fopen_s(&tokens_stream_dump, "tokens_stream_dump.txt", "a+,ccs=UNICODE");
   if (res != 0 )
   {
       fprintf(stderr, "\nError: failed to create tokens stream dump file"); exit(1);
   } 
   fwprintf(tokens_stream_dump, L"*** Begin Tokens Stream Dump ***\nThis file was automatically generated with MilkyWay lexer\n");
   fwprintf(tokens_stream_dump, L"--------------------------------------------------------------------------------\n");
}

void write_dump(char *token_name, char *token_value)
{
   fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: %hs\t\tValue: '%hs'\n",line, col, token_name, token_value);
}

void close_dump()
{
   fwprintf(tokens_stream_dump, L"--------------------------------------------------------------------------------\nDone! EOF has been reached.");
   errno_t res = fclose(tokens_stream_dump);
   if (res != 0)
   {
       fprintf(stderr, "\nError: failed to close tokens stream dump file"); exit(1);
   }
}


void dump_keywords(token_type token)
{
    switch(token)
    {
        //case tk_EOF: fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_EOF\n", line, col);break;  
        case tk_BREAK: write_dump("tk_BREAK","break"); break; 
        case tk_CASE: write_dump( "tk_CASE", "case"); break; 
        case tk_CHAN: write_dump( "tk_CHAN","chan");break; 
        case tk_CONST: write_dump( "tk_CONST","const");break;
        case tk_CONTINUE:write_dump( "tk_CONTINUE","continue");break;
        case tk_DEFAULT:write_dump( "tk_DEFAULT","default");break;
        case tk_DEFER: write_dump( "tk_DEFER","defer");break; 
        case tk_ELSE: write_dump( "tk_ELSE","else");break;
        case tk_FALLTHROUGH:write_dump( "tk_FALLTHROUGH","fallthrough");break; 
        case tk_FOR:write_dump( "tk_FOR","for");break;
        case tk_GO:write_dump( "tk_GO","go");break;
        case tk_GOTO: write_dump( "tk_GOTO","goto");break;
        case tk_IF: write_dump( "tk_IF","if");break; 
        case tk_IMPORT:write_dump( "tk_IMPORT","#include");break; 
        case tk_GOIMPORT: write_dump( "tk_GOIMPORT","goimport");break; 
        case tk_INTERFACE:write_dump( "tk_INTERFACE","interface");break; 
        case tk_MAP:write_dump( "tk_MAP","map");break; 
        case tk_PACKAGE:write_dump( "tk_PACKAGE","package");break; 
        case tk_RANGE:write_dump( "tk_RANGE","range");break;
        case tk_RETURN:write_dump( "tk_RETURN","return");break; 
        case tk_SELECT:write_dump( "tk_SELECT","select");break; 
        case tk_STRUCT:write_dump( "tk_STRUCT","struct");break; 
        case tk_SWITCH:write_dump( "tk_SWITCH","switch");break; 
        case tk_TYPE:write_dump( "tk_TYPE","type");break; 
        case tk_VAR:write_dump( "tk_VAR","var");break; 
        case tk_CLASS:write_dump( "tk_CLASS","class");break; 
        case tk_THIS:write_dump( "tk_THIS","this");break;
        case tk_EXTENDS:write_dump( "tk_EXTENDS","extends");break; 
        case tk_IMPLEMENTS:write_dump( "tk_IMPLEMENTS","implements");break; 
        case tk_NEW:write_dump( "tk_NEW","new");break; 
        case tk_SUPER:write_dump( "tk_SUPER","super");break; 
        case tk_PUBLIC:write_dump( "tk_PUBLIC","public");break; 
        case tk_PRIVATE:write_dump( "tk_PRIVATE","private");break; 
        case tk_PTRSELECT:write_dump( "tk_PTRSELECT","->");break; 
        case tk_OVERRIDE:write_dump( "tk_OVERRIDE","override");break; 
        case tk_T_STRING:write_dump( "tk_T_STRING","string");break; 
        case tk_T_BOOL:write_dump( "tk_T_BOOL","bool");break; 
        case tk_T_INT8:write_dump( "tk_T_INT8","int8");break; 
        case tk_T_UINT8:write_dump( "tk_T_UINT8","uint8");break;
        case tk_T_BYTE:write_dump( "tk_T_BYTE","byte");break; 
        case tk_T_INT16:write_dump( "tk_T_INT16","int16");break; 
        case tk_T_UINT16:write_dump( "tk_T_UINT16","uint16");break; 
        case tk_T_INT32:write_dump( "tk_T_INT32","int32");break; 
        case tk_T_UINT32:write_dump( "tk_T_UINT32","uint32");break; 
        case tk_T_RUNE:write_dump( "tk_T_RUNE","rune");break; 
        case tk_T_INT64:write_dump( "tk_T_INT64","int64");break; 
        case tk_T_UINT64:write_dump( "tk_T_UINT64","uint64");break; 
        case tk_T_INT:write_dump( "tk_T_INT","int");break; 
        case tk_T_UINTPTR:write_dump( "tk_T_UINTPTR","uintptr");break; 
        case tk_T_FLOAT32:write_dump( "tk_T_FLOAT32","float32");break; 
        case tk_T_FLOAT64:write_dump( "tk_FLOAT64","float64");break; 
        case tk_T_COMPLEX64:write_dump( "tk_T_COMPLEX64","complex64");break; 
        case tk_T_COMPLEX128:write_dump( "tk_T_COMPLEX128","complex128");break;

        case tk_TRUE: write_dump("tk_TRUE", "true");break;
        case tk_FALSE: write_dump("tk_FALSE", "false"); break;
        }
}