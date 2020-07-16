#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <sys\timeb.h> 
#define NELEMS(arr) (sizeof(arr) / sizeof(arr[0]))


#define da_dim(name, type)  type *name = NULL;          \
                            int _qy_ ## name ## _p = 0;  \
                            int _qy_ ## name ## _max = 0
#define da_rewind(name)     _qy_ ## name ## _p = 0
#define da_redim(name)      do {if (_qy_ ## name ## _p >= _qy_ ## name ## _max) \
                                name = realloc(name, (_qy_ ## name ## _max += 56) * sizeof(name[0]));} while (0)
#define da_append(name, x)  do {da_redim(name); name[_qy_ ## name ## _p++] = x;} while (0)
#define da_len(name)        _qy_ ## name ## _p

long TOKENSDUMP;

typedef enum 
{
  tk_EOF,
  /* Original keywords */ 
  tk_BREAK, tk_CASE, tk_CHAN, tk_CONST, tk_CONTINUE,
  tk_DEFAULT, tk_DEFER, tk_ELSE, tk_FALLTHROUGH, tk_FOR,
  tk_GO, tk_GOTO, tk_IF, tk_IMPORT, tk_INTERFACE, tk_MAP,
  tk_PACKAGE, tk_RANGE, tk_RETURN, tk_SELECT, tk_STRUCT, tk_SWITCH,
  tk_TYPE, tk_VAR,

  /* Up' original keywords */ 
  tk_CLASS, tk_EXTENDS, tk_IMPLEMENTS, tk_THIS, tk_NEW, tk_SUPER,
  tk_PUBLIC, tk_PRIVATE, tk_PTRSELECT, tk_OVERRIDE,
  tk_VOID, tk_INCLUDE, tk_GOINCLUDE,

  /* Original built-in types */ 
  tk_T_STRING, tk_T_BOOL, tk_T_INT8, tk_T_UINT8, tk_T_BYTE, tk_T_INT16,
  tk_T_UINT16, tk_T_INT32, tk_T_UINT32, tk_T_RUNE, tk_T_INT64, tk_T_UINT64,
  tk_T_INT, tk_T_UINT, tk_T_UINTPTR, tk_T_FLOAT32, tk_T_FLOAT64,
  tk_T_COMPLEX64, tk_T_COMPLEX128, 


  /* Original operators */ 
  tk_ADD, tk_SUB, tk_MUL, tk_DIV, tk_MOD, tk_AND, tk_OR, tk_XOR, tk_ASSIGN,
  tk_LPAREN, tk_RPAREN, tk_LSBRACKET, tk_RSBRACKET, tk_LCBRACKET, tk_RCBRACKET,
  
  /* Original punctuation */
  tk_COMMA, tk_DOT, tk_SEMI, tk_UNDERSCORE, tk_COLON,
 
  /* Original boolean operators and assigment operatoes */
  tk_LSHIFT, tk_RSHIFT, tk_EQXOR, tk_EQOR, tk_EQAND, tk_EQANDXOR, tk_EQRSHIFT, tk_EQLSHIFT,
  tk_LOGICAND, tk_LOGICOR, tk_EQADD, tk_EQSUB, tk_EQMUL, tk_EQDIV, tk_EQMOD, tk_ANDXOR, 

  /* Original relational operators */
  tk_NEG, tk_LSS, tk_GRT, tk_NOTEQ, tk_EQ, tk_EQLSS, tk_EQGRT,

   /* Original misc */
  tk_SHORTDECL, tk_ARROW, tk_INC, tk_DEC, tk_ELLIPSIS, tk_STRINGLIT, tk_NUM, tk_IDENT

} TokenType;

typedef struct 
{
 TokenType tokenType;
 int err_ln, err_col;
 union {
     int n; /* value for constants */
     char* text; /* text for idents */
 };
} Token;

static FILE *source_fp, *dest_fp;
static int line =1, col = 0, the_ch = ' ';
da_dim(text, char);

Token get_token();

static void error(int err_line, int err_col, const char *fmt, ...)
{
    char buf[1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf_s(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("(Ln %d, Col %d)  %s\n", err_line, err_col, buf);
    exit(1);
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

static Token char_lit(int n, int err_line, int err_col)
{ /* 'x' */
 if (the_ch == '\'')
    error(err_line, err_col, "Syntax error: empty character constant");
if (the_ch == '\\'){
    next_ch();
    if(the_ch == 'n')
        n = 10;
    else if (the_ch == '\\')
        n = '\\';
    else error(err_line, err_col, "Syntax error: unknown escape sequence \\%c", the_ch);
} 
if(next_ch() != '\'')
    error(err_line, err_col, "Syntax error: multi-character constant");
next_ch();
return (Token){tk_NUM, err_line, err_col, {n}};
}

static Token div_or_cmt(int err_line, int err_col) {  /* (divide)'/' or comment */
    if(the_ch != '*')
        return (Token){tk_DIV, err_line, err_col, {0}};
    if (the_ch == '/')
        printf("\nsingle comment\n");
   /* when comment found */
   next_ch();
    for (;;) {
        
        if (the_ch == '*') {
            if (next_ch() == '/') {
                next_ch();
                return get_token();
            }
        } else if (the_ch == EOF)
            error(err_line, err_col, "EOF in comment");
        else
            next_ch();
    }
}


static Token string_lit(int start, int err_line, int err_col)
{ 
    /* "string literal" */
    da_rewind(text);

    while(next_ch() != start){
        if(the_ch == '\n') error(err_line, err_col, "Syntax error: EOL in string");
        if (the_ch == EOF) error(err_line, err_col, "Syntax error: EOF in string");
        da_append(text, (char)the_ch);
    }
    da_append(text, '\0');

    next_ch();
    return (Token){tk_STRINGLIT, err_line, err_col, {.text=text}};
}

static int kwd_cmp(const void *p1, const void *p2){
    return strcmp(*(char **)p1, *(char **)p2);
}

static TokenType get_keyword_type(const char *ident)
{
    static struct{
        char *s;
        TokenType sym;
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
        {"void", tk_VOID},
        {"override", tk_OVERRIDE},
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
        {"complex128", tk_T_COMPLEX128}

    },*kwp; 
    qsort(kwds, NELEMS(kwds), sizeof(kwds[0]), kwd_cmp);

    return (kwp = bsearch(&ident, kwds, NELEMS(kwds), sizeof(kwds[0]), kwd_cmp)) == NULL ? tk_IDENT : kwp->sym;
}

int isutf8unicode(int c)
{
    return !isascii(c); 
}

static Token ident_or_num(int err_line, int err_col)
{
    int n, is_ident = false;
    da_rewind(text);
   
    while(isalnum(the_ch) || the_ch == '_' || isutf8unicode(the_ch) ){
        
        da_append(text, the_ch);
        if (!isdigit(the_ch))
            is_ident = true;
        next_ch();
    }
    
    if (da_len(text) == 0)
        error(err_line, err_col, "Syntax error: Syntax error: unrecognized character, code:  (%d) '%c'\n", the_ch, the_ch);

  
    
    da_append(text, '\0');
  
    /* only for chars that >= 1 (non-unicode chars) to avoid debug assetion failed error in isdigit() 
    and if first char of ident is digit return syntax error according to Go rule  */
        if(text[0] >=1 && isdigit(text[0])){
            if(is_ident)
                error(err_line,err_col, "Syntax error: invalid identifier, can't begin with digit '%c'", text[0], text[0]);
            n = strtol(text, NULL, 0);
            if (n == LONG_MAX && errno == ERANGE)
                error(err_line, err_col, "Syntax error: Number exceeds maximum value");
            return (Token){tk_NUM, err_line, err_col, {n}};
        } 

    return (Token) {get_keyword_type(text), err_line, err_col, {.text=text}};
}


static Token lookahead
(
    int except, 
    TokenType foundnext, 
    TokenType self, 
    int err_line, 
    int err_col
    )
{
  
    if (the_ch == except){
         next_ch();
        return (Token){foundnext, err_line, err_col, {0}};
    }
    if(self == tk_EOF)
        error(err_line, err_col, "Syntax error (lookahead): Syntax error: unrecognized character, code:  '%c' (%d)\n", the_ch, the_ch);
      
    return (Token){self, err_line, err_col, {0}};
}

static Token lookahead2
(
    int except1, 
    int except2, 
    TokenType foundl2, 
    TokenType foundl1, 
    TokenType self, 
    int err_line, 
    int err_col
)
{
    if (self == tk_EOF)
            error(err_line, err_col, "Syntax error (lookahead n2): Syntax error: unrecognized character, code:  '%c' (%d)\n", the_ch, the_ch);

    if (the_ch == except1){
        next_ch();
    
        if (the_ch == except2){
            next_ch();
            return (Token){foundl2, err_line, err_col, {0}};
        }else{
            if (foundl1 != 0)
                return (Token){foundl1, err_line, err_col, {0}};
        }
    }
          
        return (Token){self, err_line, err_col, {0}};
}


Token get_token()
{
    
    /* skip whitespace */
    while(isspace(the_ch))
        next_ch();
    int err_line = line;
    int err_col = col;
    switch(the_ch){
        case '+':
        {
            next_ch();
            Token token = lookahead('+', tk_INC, tk_ADD, err_line, err_col);
            if (token.tokenType == tk_INC)
                return token;
            token = lookahead('=', tk_EQADD, tk_ADD, err_line, err_col);
            if (token.tokenType == tk_EQADD)
                return token;
            if (token.tokenType == tk_ADD)
                return token;
        }
        case '-':
        {
            next_ch();
            Token token = lookahead('>', tk_PTRSELECT, tk_SUB, err_line, err_col);
            if (token.tokenType == tk_PTRSELECT)
                return token;
            token = lookahead('-', tk_DEC, tk_SUB, err_line, err_col);
            if (token.tokenType == tk_DEC)
                return token;
            token = lookahead('=', tk_EQSUB, tk_SUB, err_line, err_col);
            if (token.tokenType == tk_EQSUB)
                return token;
            return token; // return self, if whole lookahead false
        }
        case '*': next_ch(); return lookahead('=', tk_EQMUL, tk_MUL, err_line,err_col);
        case '/':
        {
          next_ch();
          /* skip single comment before '\n' */
          if (the_ch == '/'){ 
              do{
                  next_ch();
              }while(the_ch != '\n');
              next_ch(); return get_token();
          }

          Token token = lookahead('=', tk_EQDIV, tk_DIV, err_line, err_col);
          if (token.tokenType == tk_EQDIV)
                return token;
          /* skip multi comment before */
          return div_or_cmt(err_line, err_col); 
        }

        case '%': next_ch(); return lookahead('=', tk_EQMOD, tk_MOD, err_line, err_col);
        case '&': 
        {   
            next_ch(); 
            Token token = lookahead('=', tk_EQAND, tk_AND, err_line, err_col);
            if (token.tokenType == tk_EQAND)
                return token;
            token = lookahead('&', tk_LOGICAND, tk_AND, err_line, err_col);
            if (token.tokenType == tk_LOGICAND)
                return token;
            token = lookahead2('^', '=', tk_EQANDXOR, tk_ANDXOR, tk_AND, err_line, err_col);
            if (token.tokenType == tk_EQANDXOR || token.tokenType == tk_ANDXOR)
                return token;
            if (token.tokenType == tk_AND)
                return token;
            
        }
        case '|':
        {
            next_ch();
            Token token = lookahead('|', tk_LOGICOR, tk_OR, err_line, err_col);
            if (token.tokenType == tk_LOGICOR)
                return token;
            token = lookahead('=', tk_EQOR, tk_OR, err_line, err_col);
            if (token.tokenType == tk_EQOR)
                return token;
            if (token.tokenType == tk_OR)
                return token;
        }
        case '^': next_ch(); return lookahead('=', tk_EQXOR, tk_XOR, err_line, err_col);
        case '=': next_ch(); return lookahead('=', tk_EQ, tk_ASSIGN, err_line, err_col );
        case '(': next_ch(); return (Token){tk_LPAREN, err_line, err_col, {0}};
        case ')': next_ch(); return (Token){tk_RPAREN, err_line, err_col, {0}};
        case '[': next_ch(); return (Token){tk_LSBRACKET, err_line, err_col, {0}};
        case ']': next_ch(); return (Token){tk_RSBRACKET, err_line, err_col, {0}};
        case '{': next_ch(); return (Token){tk_LCBRACKET, err_line, err_col, {0}};
        case '}': next_ch(); return (Token){tk_RCBRACKET, err_line, err_col, {0}};
        case ',': next_ch(); return (Token){tk_COMMA, err_line, err_col, {0}};
        case '.':
        {
            next_ch();
            if (the_ch == '.'){
                next_ch();
                if (the_ch == '.'){
                    next_ch();
                    return (Token){tk_ELLIPSIS, err_line,err_col,{0}};
                }else 
                   error(err_line, err_col, "Syntax error (lookahead): unrecognized character, code:  .\n", the_ch, the_ch);
                
            }
            return (Token){tk_DOT, err_line, err_col, {0}};

        } 
        case ';': next_ch(); return (Token){tk_SEMI, err_line, err_col, {0}};
        case '_': next_ch(); return (Token){tk_UNDERSCORE, err_line, err_col, {0}};
        case ':': next_ch(); return lookahead('=', tk_SHORTDECL, tk_SEMI, err_line, err_col);
        case '!': next_ch(); return lookahead('=', tk_NOTEQ, tk_NEG, err_line, err_col);
        case '<':
        {
            next_ch();
            Token token = lookahead('=', tk_EQLSS, tk_LSS, err_line, err_col);
            if (token.tokenType == tk_EQLSS)
                return token;
            token = lookahead('-', tk_ARROW, tk_LSS, err_line, err_col);
            if (token.tokenType == tk_ARROW)
                return token;
            token = lookahead2('<', '=', tk_EQLSHIFT, tk_LSHIFT, tk_LSS, err_line, err_col);
            if (token.tokenType == tk_EQLSHIFT || token.tokenType == tk_LSHIFT)
                return token;
            if (token.tokenType == tk_LSS)
                return token;

        } 
        case '>':
        {
            next_ch();
            Token token = lookahead('=', tk_EQGRT, tk_GRT, err_line, err_col);
            if (token.tokenType == tk_EQGRT)
                return token;
            token = lookahead2('>', '=', tk_RSHIFT, tk_EQRSHIFT, tk_GRT , err_line, err_col );
            if (token.tokenType == tk_RSHIFT || token.tokenType == tk_EQRSHIFT)
                return token;
            if (token.tokenType == tk_GRT)
                return token;
        }
        case '"': return string_lit(the_ch, err_line, err_col);
        case '#':
        {
            next_ch();
            ident_or_num(err_line, err_col); 
            if (strcmp(text, "goinclude") != 0 && strcmp(text, "include") !=0 )
                error(err_line, err_col, "Syntax error: invalid include statement, expected 'include' or 'goinclude' after '#'");
            if (strcmp(text, "goinclude") == 0)
                return (Token){tk_GOINCLUDE, err_line, err_col, {0}};
            if (strcmp(text, "include") == 0)
                return (Token){tk_INCLUDE, err_line, err_col, {0}};
            error(err_line, err_col, "Syntax error: unrecognized character (%d) '%c'", the_ch, the_ch);
        }
         
        // case '\'': next_ch(); return char_lit(the_ch, err_line, err_col);

        default: return ident_or_num(err_line, err_col);
        case EOF: return (Token){tk_EOF, err_line, err_col, {0}}; // WHATTTTTTTTTTT????
    }
}

FILE *tokens_stream_dump;

void create_dump()
{
   errno_t res = fopen_s(&tokens_stream_dump, "tokens_stream_dump.txt", "a+,ccs=UNICODE");
   if (res != 0 )
   {
       fprintf(stderr, "\nError: failed to create tokens stream dump file"); exit(1);
   } 
}

void write_dump(char *token_name, char *token_value, int err_line, int err_col)
{
   fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: %hs\t\tValue: %hs\n",err_line, err_col, token_name, token_value);
}

void close_dump()
{
   errno_t res = fclose(tokens_stream_dump);
   if (res != 0)
   {
       fprintf(stderr, "\nError: failed to close tokens stream dump file"); exit(1);
   }
}


void start_dump(Token token)
{
    switch(token.tokenType)
    {
        case tk_EOF: fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_EOF\nDone! EOF has been reached\n",token.err_ln, token.err_col);break;  
        case tk_BREAK: write_dump("tk_BREAK","break",token.err_ln, token.err_col); break; 
        case tk_CASE: write_dump( "tk_CASE", "case", token.err_ln, token.err_col); break; 
        case tk_CHAN: write_dump( "tk_CHAN","chan", token.err_ln, token.err_col);break; 
        case tk_CONST: write_dump( "tk_CONST","const", token.err_ln, token.err_col);break;
        case tk_CONTINUE:write_dump( "tk_CONTINUE","continue", token.err_ln, token.err_col);break;
        case tk_DEFAULT:write_dump( "tk_DEFAULT","default", token.err_ln, token.err_col);break;
        case tk_DEFER: write_dump( "tk_DEFER","defer", token.err_ln, token.err_col);break; 
        case tk_ELSE: write_dump( "tk_ELSE","else", token.err_ln, token.err_col);break;
        case tk_FALLTHROUGH:write_dump( "tk_FALLTHROUGH","fallthrough", token.err_ln, token.err_col);break; 
        case tk_FOR:write_dump( "tk_FOR","for", token.err_ln, token.err_col);break;
        case tk_GO:write_dump( "tk_GO","go", token.err_ln, token.err_col);break;
        case tk_GOTO: write_dump( "tk_GOTO","goto", token.err_ln, token.err_col);break;
        case tk_IF: write_dump( "tk_IF","if", token.err_ln, token.err_col);break; 
        case tk_VOID: write_dump( "tk_VOID","void", token.err_ln, token.err_col);break; 
        case tk_INCLUDE:write_dump( "tk_INCLUDE","#include", token.err_ln, token.err_col);break; 
        case tk_GOINCLUDE: write_dump( "tk_GOINCLUDE","#goinclude", token.err_ln, token.err_col);break; 
        case tk_INTERFACE:write_dump( "tk_INTERFACE","interface", token.err_ln, token.err_col);break; 
        case tk_MAP:write_dump( "tk_MAP","map", token.err_ln, token.err_col);break; 
        case tk_PACKAGE:write_dump( "tk_PACKAGE","package", token.err_ln, token.err_col);break; 
        case tk_RANGE:write_dump( "tk_RANGE","range", token.err_ln, token.err_col);break;
        case tk_RETURN:write_dump( "tk_RETURN","return", token.err_ln, token.err_col);break; 
        case tk_SELECT:write_dump( "tk_SELECT","select", token.err_ln, token.err_col);break; 
        case tk_STRUCT:write_dump( "tk_STRUCT","struct", token.err_ln, token.err_col);break; 
        case tk_SWITCH:write_dump( "tk_SWITCH","switch", token.err_ln, token.err_col);break; 
        case tk_TYPE:write_dump( "tk_TYPE","type", token.err_ln, token.err_col);break; 
        case tk_VAR:write_dump( "tk_VAR","var", token.err_ln, token.err_col);break; 
        case tk_CLASS:write_dump( "tk_CLASS","class", token.err_ln, token.err_col);break; 
        case tk_THIS:write_dump( "tk_THIS","this", token.err_ln, token.err_col);break;
        case tk_EXTENDS:write_dump( "tk_EXTENDS","extends", token.err_ln, token.err_col);break; 
        case tk_IMPLEMENTS:write_dump( "tk_IMPLEMENTS","implements", token.err_ln, token.err_col);break; 
        case tk_NEW:write_dump( "tk_NEW","new", token.err_ln, token.err_col);break; 
        case tk_SUPER:write_dump( "tk_SUPER","super", token.err_ln, token.err_col);break; 
        case tk_PUBLIC:write_dump( "tk_PUBLIC","public", token.err_ln, token.err_col);break; 
        case tk_PRIVATE:write_dump( "tk_PRIVATE","private", token.err_ln, token.err_col);break; 
        case tk_PTRSELECT:write_dump( "tk_PTRSELECT","->", token.err_ln, token.err_col);break; 
        case tk_OVERRIDE:write_dump( "tk_OVERRIDE","override", token.err_ln, token.err_col);break; 
        case tk_T_STRING:write_dump( "tk_T_STRING","string", token.err_ln, token.err_col);break; 
        case tk_T_BOOL:write_dump( "tk_T_BOOL","bool", token.err_ln, token.err_col);break; 
        case tk_T_INT8:write_dump( "tk_T_INT8","int8", token.err_ln, token.err_col);break; 
        case tk_T_UINT8:write_dump( "tk_T_UINT8","uint8", token.err_ln, token.err_col);break;
        case tk_T_BYTE:write_dump( "tk_T_BYTE","byte", token.err_ln, token.err_col);break; 
        case tk_T_INT16:write_dump( "tk_T_INT16","int16", token.err_ln, token.err_col);break; 
        case tk_T_UINT16:write_dump( "tk_T_UINT16","uint16", token.err_ln, token.err_col);break; 
        case tk_T_INT32:write_dump( "tk_T_INT32","int32", token.err_ln, token.err_col);break; 
        case tk_T_UINT32:write_dump( "tk_T_UINT32","uint32", token.err_ln, token.err_col);break; 
        case tk_T_RUNE:write_dump( "tk_T_RUNE","rune", token.err_ln, token.err_col);break; 
        case tk_T_INT64:write_dump( "tk_T_INT64","int64", token.err_ln, token.err_col);break; 
        case tk_T_UINT64:write_dump( "tk_T_UINT64","uint64", token.err_ln, token.err_col);break; 
        case tk_T_INT:write_dump( "tk_T_INT","int", token.err_ln, token.err_col);break; 
        case tk_T_UINTPTR:write_dump( "tk_T_UINTPTR","uintptr", token.err_ln, token.err_col);break; 
        case tk_T_FLOAT32:write_dump( "tk_T_FLOAT32","float32", token.err_ln, token.err_col);break; 
        case tk_T_FLOAT64:write_dump( "tk_FLOAT64","float64", token.err_ln, token.err_col);break; 
        case tk_T_COMPLEX64:write_dump( "tk_T_COMPLEX64","complex64", token.err_ln, token.err_col);break; 
        case tk_T_COMPLEX128:write_dump( "tk_T_COMPLEX128","complex128", token.err_ln, token.err_col);break;
        case tk_ADD:write_dump( "tk_ADD","+", token.err_ln, token.err_col);break;
        case tk_SUB:write_dump( "tk_SUB","-", token.err_ln, token.err_col);break; 
        case tk_MUL:write_dump( "tk_MUL","*", token.err_ln, token.err_col);break; 
        case tk_DIV:write_dump( "tk_DIV","/", token.err_ln, token.err_col);break; 
        case tk_MOD:write_dump( "tk_MOD","%", token.err_ln, token.err_col);break; 
        case tk_AND:write_dump( "tk_AND","&", token.err_ln, token.err_col);break; 
        case tk_OR:write_dump( "tk_OR","|", token.err_ln, token.err_col);break; 
        case tk_XOR:write_dump( "tk_XOR","^", token.err_ln, token.err_col);break; 
        case tk_ASSIGN:write_dump( "tk_ASSIGN","=", token.err_ln, token.err_col);break; 
        case tk_LPAREN:write_dump( "tk_LPAREN","(", token.err_ln, token.err_col);break; 
        case tk_RPAREN:write_dump( "tk_RPAREN",")", token.err_ln, token.err_col);break; 
        case tk_LSBRACKET:write_dump( "tk_LSBRACKET","[", token.err_ln, token.err_col);break; 
        case tk_RSBRACKET:write_dump( "tk_RSBRACKET","]", token.err_ln, token.err_col);break; 
        case tk_LCBRACKET:write_dump( "tk_LCBRACKET","{", token.err_ln, token.err_col);break; 
        case tk_RCBRACKET:write_dump( "tk_RCBRACKET","}", token.err_ln, token.err_col);break; 
        case tk_COMMA:write_dump( "tk_COMMA",",", token.err_ln, token.err_col);break; 
        case tk_DOT:write_dump( "tk_DOT",".", token.err_ln, token.err_col);break; 
        case tk_SEMI:write_dump( "tk_SEMI",";", token.err_ln, token.err_col);break; 
        case tk_UNDERSCORE:write_dump( "tk_UNDERSCORE","_", token.err_ln, token.err_col);break; 
        case tk_COLON:write_dump( "tk_COLON",":", token.err_ln, token.err_col);break; 
        case tk_LSHIFT:write_dump( "tk_LSHIFT","<<", token.err_ln, token.err_col);break; 
        case tk_RSHIFT:write_dump( "tk_RSHIFT",">>", token.err_ln, token.err_col);break;
        case tk_EQXOR:write_dump( "tk_EQXOR","^=", token.err_ln, token.err_col);break; 
        case tk_EQOR: write_dump( "tk_EQOR","|=", token.err_ln, token.err_col);break;
        case tk_EQAND:write_dump( "tk_EQAND","&=", token.err_ln, token.err_col);break; 
        case tk_EQANDXOR:write_dump( "tk_EQANDXOR","&^=", token.err_ln, token.err_col);break; 
        case tk_EQRSHIFT:write_dump( "tk_EQRSHIFT",">>=", token.err_ln, token.err_col);break; 
        case tk_LOGICAND:write_dump( "tk_LOGICAND","&&", token.err_ln, token.err_col);break; 
        case tk_EQLSHIFT:write_dump( "tk_EQLSHIFT","<<=", token.err_ln, token.err_col);break; 
        case tk_LOGICOR:write_dump( "tk_LOGICOR","||", token.err_ln, token.err_col);break;
        case tk_EQADD:write_dump( "tk_EQADD","+=", token.err_ln, token.err_col);break;
        case tk_EQSUB:write_dump( "tk_EQSUB","-=", token.err_ln, token.err_col);break; 
        case tk_EQMUL:write_dump( "tk_EQMUL","*=", token.err_ln, token.err_col);break; 
        case tk_EQDIV:write_dump( "tk_EQDIV","/=", token.err_ln, token.err_col);break;
        case tk_EQMOD:write_dump( "tk_EQMOD","%=", token.err_ln, token.err_col);break; 
        case tk_ANDXOR:write_dump( "tk_ANDXOR","&^", token.err_ln, token.err_col);break; 
        case tk_NEG:write_dump( "tk_NEG","!", token.err_ln, token.err_col);break; 
        case tk_LSS:write_dump( "tk_LSS","<", token.err_ln, token.err_col);break;
        case tk_GRT:write_dump( "tk_GRT",">", token.err_ln, token.err_col);break; 
        case tk_NOTEQ:write_dump( "tk_NOTEQ","!=", token.err_ln, token.err_col);break; 
        case tk_EQ:write_dump( "tk_EQ","==", token.err_ln, token.err_col);break; 
        case tk_EQLSS:write_dump( "tk_EQLSS","<=", token.err_ln, token.err_col);break; 
        case tk_EQGRT:write_dump( "tk_EQGRT",">=", token.err_ln, token.err_col);break; 
        case tk_SHORTDECL:write_dump( "tk_SHORTDECL",":=", token.err_ln, token.err_col);break; 
        case tk_ARROW:write_dump( "tk_ARROW","<-", token.err_ln, token.err_col);break; 
        case tk_INC:write_dump( "tk_INC","++", token.err_ln, token.err_col);break; 
        case tk_DEC:write_dump( "tk_DEC","--", token.err_ln, token.err_col);break; 
        case tk_ELLIPSIS:write_dump( "tk_ELLIPSIS","...", token.err_ln, token.err_col);break;
        case tk_NUM: fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_NUM\t\tSemanticValue: %d\n", token.err_ln, token.err_col, token.n); break;     
        case tk_STRINGLIT: fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_STRINGLIT\t\tSemanticValue: %hs\n", token.err_ln, token.err_col, token.text); break;
        case tk_IDENT:  fwprintf(tokens_stream_dump, L"Source: Ln %d, Col %d\t\tToken: tk_IDENT\t\tSemanticValue: %hs\n", token.err_ln, token.err_col, token.text); break;
        }
}

void lex()
{
    create_dump();
    Token token;
    struct timeb start, end;
    ftime(&start);
    do{
        token = get_token();

    if (TOKENSDUMP == 1)
        start_dump(token);

    }while(token.tokenType != tk_EOF);

    ftime(&end);
    int diff = (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm));
    close_dump();
    printf("\nLexing time: %u milliseconds = %u microseconds", diff, diff*1000);
    fclose(source_fp);
}

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