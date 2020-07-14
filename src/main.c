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

long LEXOUTPUT;

typedef enum {
  tk_EOF,
  /* Original keywords */ 
  tk_BREAK, tk_CASE, tk_CHAN, tk_CONST, tk_CONTINUE,
  tk_DEFAULT, tk_DEFER, tk_ELSE, tk_FALLTHROUGH, tk_FOR,
  tk_FUNC, tk_GO, tk_GOTO, tk_IF, tk_IMPORT, tk_INTERFACE, tk_MAP,
  tk_PACKAGE, tk_RANGE, tk_RETURN, tk_SELECT, tk_STRUCT, tk_SWITCH,
  tk_TYPE, tk_VAR,

  /* GoToClass' original keywords */ 
  tk_CLASS, tk_EXTENDS, tk_IMPLEMENTS, tk_THIS, tk_NEW, tk_SUPER,
  tk_PUBLIC, tk_PRIVATE, tk_INSTANCEOF, tk_PTRSELECT, tk_OVERRIDE,

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
  tk_SHORTDECL, tk_ARROW, tk_INC, tk_DEC, tk_ELLIPSIS, tk_STRINGLIT, tk_NUM, tk_SYM

} TokenType;

typedef struct {
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

static void error(int err_line, int err_col, const char *fmt, ...){
    char buf[1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf_s(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("(%d, %d) error: %s\n", err_line, err_col, buf);
    exit(1);
}

static int next_ch(){  /* get next char from our input */
    the_ch = getc(source_fp);
   
    ++col;
    if(the_ch == '\n'){
        ++line;
        col = 0;
    }
    return the_ch;
}


static int back_ch(int ch){
    ungetc(ch , source_fp);
}

static Token char_lit(int n, int err_line, int err_col){ /* 'x' */
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


static Token string_lit(int start, int err_line, int err_col){ /* "string literal" */
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

static TokenType get_keyword_type(const char *ident){
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
        {"func", tk_FUNC},
        {"goto", tk_GOTO},
        {"go", tk_GO},
        {"if", tk_IF},
        {"import", tk_IMPORT},
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
        {"instanceof", tk_INSTANCEOF},
        //{"->", tk_ARROW},
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

    return (kwp = bsearch(&ident, kwds, NELEMS(kwds), sizeof(kwds[0]), kwd_cmp)) == NULL ? tk_SYM : kwp->sym;
}

int isutf8unicode(int c)
{
    return !isascii(c); 
}

static Token sym_or_num(int err_line, int err_col)
{
    int n, is_number = true;
    da_rewind(text);
    while(isalnum(the_ch) || the_ch == '_' || isutf8unicode(the_ch) ){
        da_append(text, the_ch);
        if (!isdigit(the_ch))
            is_number = false;
        next_ch();
    }
    if (da_len(text) == 0)
        error(err_line, err_col, "Syntax error: Syntax error: unrecognized character, code:  (%d) '%c'\n", the_ch, the_ch);
    
    da_append(text, '\0');
            
    if(isdigit(text[0])){
        if(!is_number)
            error(err_line, err_col, "Syntax error: invalid number: %s\n", text);
        n = strtol(text, NULL, 0);
        if (n == LONG_MAX && errno == ERANGE)
            error(err_line, err_col, "Syntax error: Number exceeds maximum value");
        return (Token){tk_NUM, err_line, err_col, {n}};
    }
    return (Token) {get_keyword_type(text), err_line, err_col, {.text=text}};
}


static Token lookahead(int except, TokenType foundnext, TokenType self, int err_line, int err_col){
  
    if (the_ch == except){
         next_ch();
        return (Token){foundnext, err_line, err_col, {0}};
    }
    if(self == tk_EOF)
        error(err_line, err_col, "Syntax error (lookahead): Syntax error: unrecognized character, code:  '%c' (%d)\n", the_ch, the_ch);
      
    return (Token){self, err_line, err_col, {0}};
}

static Token lookahead2(int except1, int except2, TokenType foundl2, TokenType foundl1, TokenType self, int err_line, int err_col){
  
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


Token get_token(){
    
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
         
        // case '\'': next_ch(); return char_lit(the_ch, err_line, err_col);

        default: return sym_or_num(err_line, err_col);
        case EOF: return (Token){tk_EOF, err_line, err_col, {0}}; // WHATTTTTTTTTTT????
    }
}


void lexPrint(Token token){
    
        switch(token.tokenType){
           
            case tk_EOF: printf("Source: Ln %d, Col %d\t\tToken: tk_EOF\n\nDone! EOF has been reached.", token.err_ln, token.err_col); break;
            case tk_BREAK: printf("Source: Ln %d, Col %d\t\tToken: tk_BREAK\t\tValue: break\n", token.err_ln, token.err_ln); break;
            case tk_CASE: printf("Source: Ln %d, Col %d\t\tToken: tk_CASE\t\tValue: case\n", token.err_ln, token.err_col); break;
            case tk_CHAN: printf("Source: Ln %d, Col %d\t\tToken: tk_CHAN\t\tValue: chan\n", token.err_ln, token.err_col); break;
            case tk_CONST: printf("Source: Ln %d, Col %d\t\tToken: tk_CONST\t\tValue: const\n", token.err_ln, token.err_col); break;
            case tk_CONTINUE: printf("Source: Ln %d, Col %d\t\tToken: tk_CONTINUE\t\tValue: continue\n", token.err_ln, token.err_col); break;
            case tk_DEFAULT: printf("Source: Ln %d, Col %d\t\tToken: tk_DEFAULT\t\tValue: default\n", token.err_ln, token.err_col); break;
            case tk_DEFER: printf("Source: Ln %d, Col %d\t\tToken: tk_DEFER\t\tValue: defer\n", token.err_ln, token.err_col); break;
            case tk_ELSE: printf("Source: Ln %d, Col %d\t\tToken: tk_ELSE\t\tValue: else\n", token.err_ln, token.err_col); break;
            case tk_FALLTHROUGH: printf("Source: Ln %d, Col %d\t\tToken: tk_FALLTHROUGH\t\tValue: fallthrough\n", token.err_ln, token.err_col); break;
            case tk_FOR: printf("Source: Ln %d, Col %d\t\tToken: tk_FOR\t\tValue: for\n", token.err_ln, token.err_col); break;
            case tk_FUNC: printf("Source: Ln %d, Col %d\t\tToken: tk_FUNC\t\tValue: func\n", token.err_ln, token.err_col); break;
            case tk_GO: printf("Source: Ln %d, Col %d\t\tToken: tk_GO\t\tValue: go\n", token.err_ln, token.err_col); break;
            case tk_GOTO: printf("Source: Ln %d, Col %d\t\tToken: tk_GOTO\t\tValue: goto\n", token.err_ln, token.err_col); break;
            case tk_IF: printf("Source: Ln %d, Col %d\t\tToken: tk_IF\t\tValue: if\n", token.err_ln, token.err_col); break;
            case tk_IMPORT: printf("Source: Ln %d, Col %d\t\tToken: tk_IMPORT\t\tValue: import\n", token.err_ln, token.err_col); break;
            case tk_INTERFACE: printf("Source: Ln %d, Col %d\t\tToken: tk_INTERFACE\t\tValue: interface\n", token.err_ln, token.err_col); break;
            case tk_MAP: printf("Source: Ln %d, Col %d\t\tToken: tk_MAP\t\tValue: map\n", token.err_ln, token.err_col); break;
            case tk_PACKAGE: printf("Source: Ln %d, Col %d\t\tToken: tk_PACKAGE\t\tValue: package\n", token.err_ln, token.err_col); break;
            case tk_RANGE: printf("Source: Ln %d, Col %d\t\tToken: tk_RANGE\t\tValue: range\n", token.err_ln, token.err_col); break;
            case tk_RETURN: printf("Source: Ln %d, Col %d\t\tToken: tk_RETURN\t\tValue: return\n", token.err_ln, token.err_col); break;
            case tk_SELECT: printf("Source: Ln %d, Col %d\t\tToken: tk_SELECT\t\tValue: select\n", token.err_ln, token.err_col); break;
            case tk_STRUCT: printf("Source: Ln %d, Col %d\t\tToken: tk_STRUCT\t\tValue: struct\n", token.err_ln, token.err_col); break;
            case tk_SWITCH: printf("Source: Ln %d, Col %d\t\tToken: tk_SWITCH\t\tValue: switch\n", token.err_ln, token.err_col); break;
            case tk_TYPE: printf("Source: Ln %d, Col %d\t\tToken: tk_TYPE\t\tValue: type\n", token.err_ln, token.err_col); break;
            case tk_VAR: printf("Source: Ln %d, Col %d\t\tToken: tk_VAR\t\tValue: var\n", token.err_ln, token.err_col); break;
            case tk_CLASS: printf("Source: Ln %d, Col %d\t\tToken: tk_CLASS\t\tValue: class\n", token.err_ln, token.err_col); break;
            case tk_THIS: printf("Source: Ln %d, Col %d\t\tToken: tk_THIS\t\tValue: this\n", token.err_ln, token.err_col); break;
            case tk_EXTENDS: printf("Source: Ln %d, Col %d\t\tToken: tk_EXTENDS\t\tValue: extends\n", token.err_ln, token.err_col); break;
            case tk_IMPLEMENTS: printf("Source: Ln %d, Col %d\t\tToken: tk_IMPLEMENTS\t\tValue: implements\n", token.err_ln, token.err_col); break;
            case tk_NEW: printf("Source: Ln %d, Col %d\t\tToken: tk_NEW\t\tValue: new\n", token.err_ln, token.err_col); break;
            case tk_SUPER: printf("Source: Ln %d, Col %d\t\tToken: tk_SUPER\t\tValue: super\n", token.err_ln, token.err_col); break;
            case tk_PUBLIC: printf("Source: Ln %d, Col %d\t\tToken: tk_PUBLIC\t\tValue: public\n", token.err_ln, token.err_col);  break;
            case tk_PRIVATE: printf("Source: Ln %d, Col %d\t\tToken: tk_PRIVATE\t\tValue: private\n", token.err_ln, token.err_col); break;
            case tk_INSTANCEOF: printf("Source: Ln %d, Col %d\t\tToken: tk_INSTANCEOF\t\tValue: instanceof\n", token.err_ln, token.err_col ); break;
            case tk_PTRSELECT: printf("Source: Ln %d, Col %d\t\tToken: tk_PTRSELECT\t\tValue: ->\n", token.err_ln, token.err_col); break;
            case tk_OVERRIDE: printf("Source: Ln %d, Col %d\t\tToken: tk_OVERRIDE\t\tValue: override\n", token.err_ln, token.err_col); break;
            case tk_T_STRING: printf("Source: Ln %d, Col %d\t\tToken: tk_T_STRING\t\tValue: string\n", token.err_ln, token.err_col); break;
            case tk_T_BOOL: printf("Source: Ln %d, Col %d\t\tToken: tk_T_BOOL\t\tValue: bool\n", token.err_ln, token.err_col); break;
            case tk_T_INT8: printf("Source: Ln %d, Col %d\t\tToken: tk_T_INT8\t\tValue: int8\n", token.err_ln, token.err_col); break;
            case tk_T_UINT8: printf("Source: Ln %d, Col %d\t\tToken: tk_T_UINT8\t\tValue: uint8\n", token.err_ln, token.err_col); break;
            case tk_T_BYTE: printf("Source: Ln %d, Col %d\t\tToken: tk_T_BYTE\t\tValue: byte\n", token.err_ln, token.err_col); break;
            case tk_T_INT16: printf("Source: Ln %d, Col %d\t\tToken: tk_T_INT16\t\tValue: int16\n", token.err_ln, token.err_col); break;
            case tk_T_UINT16: printf("Source: Ln %d, Col %d\t\tToken: tk_T_UINT16\t\tValue: uint16\n", token.err_ln, token.err_col); break;
            case tk_T_INT32: printf("Source: Ln %d, Col %d\t\tToken: tk_T_INT32\t\tValue: int32\n", token.err_ln, token.err_col); break;
            case tk_T_UINT32: printf("Source: Ln %d, Col %d\t\tToken: tk_T_UINT32\t\tValue: uint32\n",token.err_ln, token.err_col); break;
            case tk_T_RUNE: printf("Source: Ln %d, Col %d\t\tToken: tk_T_RUNE\t\tValue: rune\n", token.err_ln, token.err_col); break;
            case tk_T_INT64: printf("Source: Ln %d, Col %d\t\tToken: tk_T_INT64\t\tValue: int64\n", token.err_ln, token.err_col); break;
            case tk_T_UINT64: printf("Source: Ln %d, Col %d\t\tToken: tk_T_UINT64\t\tValue: uint64\n", token.err_ln, token.err_col); break;
            case tk_T_INT: printf("Source: Ln %d, Col %d\t\tToken: tk_T_INT\t\tValue: int\n", token.err_ln, token.err_col); break;
            case tk_T_UINTPTR: printf("Source: Ln %d, Col %d\t\tToken: tk_UINTPTR\t\tValue: uintptr\n", token.err_ln, token.err_col); break;
            case tk_T_FLOAT32: printf("Source: Ln %d, Col %d\t\tToken: tk_T_FLOAT32\t\tValue: float32\n", token.err_ln, token.err_col); break;
            case tk_T_FLOAT64: printf("Source: Ln %d, Col %d\t\tToken: tk_T_FLOAT64\t\tValue: float64\n", token.err_ln, token.err_col); break;
            case tk_T_COMPLEX64: printf("Source: Ln %d, Col %d\t\tToken: tk_T_COMPLEX64\t\tValue: complex64\n", token.err_ln, token.err_col); break;
            case tk_T_COMPLEX128: printf("Source: Ln %d, Col %d\t\tToken: tk_T_COMPLEX128\t\tValue: complex128\n", token.err_ln, token.err_col); break;
            case tk_ADD: printf("Source: Ln %d, Col %d\t\tToken: tk_ADD\t\tValue: +\n", token.err_ln, token.err_col); break;
            case tk_SUB: printf("Source: Ln %d, Col %d\t\tToken: tk_SUB\t\tValue: -\n", token.err_ln, token.err_col); break;
            case tk_MUL: printf("Source: Ln %d, Col %d\t\tToken: tk_MUL\t\tValue: *\n", token.err_ln, token.err_col); break;
            case tk_DIV: printf("Source: Ln %d, Col %d\t\tToken: tk_DIV\t\tValue: /\n", token.err_ln, token.err_col); break;
            case tk_MOD: printf("Source: Ln %d, Col %d\t\tToken: tk_MOD\t\tValue: 'todo'\n", token.err_ln, token.err_col); break;
            case tk_AND: printf("Source: Ln %d, Col %d\t\tToken: tk_AND\t\tValue: &\n", token.err_ln, token.err_col); break;
            case tk_OR: printf("Source: Ln %d, Col %d\t\tToken: tk_OR\t\tValue: |\n", token.err_ln, token.err_col); break;
            case tk_XOR: printf("Source: Ln %d, Col %d\t\tToken: tk_XOR\t\tValue: ^\n", token.err_ln, token.err_col); break;
            case tk_ASSIGN: printf("Source: Ln %d, Col %d\t\tToken: tk_ASSIGN\t\tValue: =\n", token.err_ln, token.err_col); break;
            case tk_LPAREN: printf("Source: Ln %d, Col %d\t\tToken: tk_LPAREN\t\tValue: (\n", token.err_ln, token.err_col); break;
            case tk_RPAREN: printf("Source: Ln %d, Col %d\t\tToken: tk_RPAREN\t\tValue: )\n", token.err_ln, token.err_col); break;
            case tk_LSBRACKET: printf("Source: Ln %d, Col %d\t\tToken: tk_LSBRACKET\t\tValue: [\n", token.err_ln, token.err_col); break;
            case tk_RSBRACKET: printf("Source: Ln %d, Col %d\t\tToken: tk_RSBRACKET\t\tValue:]\n", token.err_ln, token.err_col); break;
            case tk_LCBRACKET: printf("Source: Ln %d, Col %d\t\tToken: tk_LCBRACKET\t\tValue: {\n", token.err_ln, token.err_col); break;
            case tk_RCBRACKET: printf("Source: Ln %d, Col %d\t\tToken: tk_RCBRACKET\t\tValue: }\n", token.err_ln, token.err_col); break;
            case tk_COMMA: printf("Source: Ln %d, Col %d\t\tToken: tk_COMMA\t\tValue: ,\n", token.err_ln, token.err_col); break;
            case tk_DOT: printf("Source: Ln %d, Col %d\t\tToken: tk_DOT\t\tValue: .\n", token.err_ln, token.err_col); break;
            case tk_SEMI: printf("Source: Ln %d, Col %d\t\tToken: tk_SEMI\t\tValue: ;\n", token.err_ln, token.err_col); break;
            case tk_UNDERSCORE: printf("Source: Ln %d, Col %d\t\tToken: tk_UNDERSCORE\t\tValue: _\n", token.err_ln, token.err_col); break;
            case tk_COLON: printf("Source: Ln %d, Col %d\t\tToken: tk_COLON\t\tValue: :\n", token.err_ln, token.err_col); break;
            case tk_LSHIFT: printf("Source: Ln %d, Col %d\t\tToken: tk_LSHIFT\t\tValue: <<\n", token.err_ln, token.err_col); break;
            case tk_RSHIFT: printf("Source: Ln %d, Col %d\t\tToken: tk_RSHIFT\t\tValue: >>\n", token.err_ln, token.err_col); break;
            case tk_EQXOR: printf("Source: Ln %d, Col %d\t\tToken: tk_EQXOR\t\tValue: ^=\n", token.err_ln, token.err_col); break;
            case tk_EQOR: printf("Source: Ln %d, Col %d\t\tToken: tk_EQOR\t\tValue: |=\n", token.err_ln, token.err_col); break;
            case tk_EQAND: printf("Source: Ln %d, Col %d\t\tToken: tk_EQAND\t\tValue: &=\n", token.err_ln, token.err_col); break;
            case tk_EQANDXOR: printf("Source: Ln %d, Col %d\t\tToken: tk_EQANDXOR\t\tValue: &^=\n", token.err_ln, token.err_col); break;
            case tk_EQRSHIFT: printf("Source: Ln %d, Col %d\t\tToken: tk_EQRSHIFT\t\tValue: >>=\n", token.err_ln, token.err_col); break;
            case tk_LOGICAND: printf("Source: Ln %d, Col %d\t\tToken: tk_LOGICKAND\t\tValue: &&\n", token.err_ln, token.err_col); break;
            case tk_EQLSHIFT: printf("Source: Ln %d, Col %d\t\tToken: tk_EQLSHIFT\t\tValue: <<=\n", token.err_ln, token.err_col); break;
            case tk_LOGICOR: printf("Source: Ln %d, Col %d\t\tToken: tk_LOGICOR\t\tValue: ||\n", token.err_ln, token.err_col); break;
            case tk_EQADD: printf("Source: Ln %d, Col %d\t\tToken: tk_EQADD\t\tValue: +=\n", token.err_ln, token.err_col); break;
            case tk_EQSUB: printf("Source: Ln %d, Col %d\t\tToken: tk_EQSUB\t\tValue: -=\n", token.err_ln, token.err_col); break;
            case tk_EQMUL: printf("Source: Ln %d, Col %d\t\tToken: tk_EQMUL\t\tValue: *=\n", token.err_ln, token.err_col); break;
            case tk_EQDIV: printf("Source: Ln %d, Col %d\t\tToken: tk_EQDIV\t\tValue: /=\n", token.err_ln, token.err_col); break;
            case tk_EQMOD: printf("Source: Ln %d, Col %d\t\tToken: tk_EQMOD\t\tValue: amp=\n", token.err_ln, token.err_col); break;
            case tk_ANDXOR: printf("Source: Ln %d, Col %d\t\tToken: tk_ANDXOR\t\tValue: &^\n", token.err_ln, token.err_col); break;
            case tk_NEG: printf("Source: Ln %d, Col %d\t\tToken: tk_NEG\t\tValue: !\n", token.err_ln, token.err_col ); break;
            case tk_LSS: printf("Source: Ln %d, Col %d\t\tToken: tk_LSS\t\tValue: <\n", token.err_ln, token.err_col); break;
            case tk_GRT: printf("Source: Ln %d, Col %d\t\tToken: tk_GRT\t\tValue: >\n", token.err_ln, token.err_col); break;
            case tk_NOTEQ: printf("Source: Ln %d, Col %d\t\tToken: tk_NOTEQ\t\tValue: !=\n", token.err_ln, token.err_col); break;
            case tk_EQ: printf("Source: Ln %d, Col %d\t\tToken: tk_EQ\t\tValue: ==\n", token.err_ln, token.err_col ); break;
            case tk_EQLSS: printf("Source: Ln %d, Col %d\t\tToken: tk_EQLSS\t\tValue: <=\n", token.err_ln, token.err_col); break;
            case tk_EQGRT: printf("Source: Ln %d, Col %d\t\tToken: tk_EQGRT\t\tValue: >=\n", token.err_ln, token.err_col); break;
            case tk_SHORTDECL: printf("Source: Ln %d, Col %d\t\tToken: tk_SHORTDECL\t\tValue: :=\n", token.err_ln, token.err_col); break;
            case tk_ARROW: printf("Source: Ln %d, Col %d\t\tToken: tk_ARROW\t\tValue: <-\n", token.err_ln, token.err_col); break;
            case tk_INC: printf("Source: Ln %d, Col %d\t\tToken: tk_INC\t\tValue: ++\n", token.err_ln, token.err_col); break;
            case tk_DEC: printf("Source: Ln %d, Col %d\t\tToken: tk_DEC\t\tValue: --\n", token.err_ln, token.err_col); break;
            case tk_ELLIPSIS: printf("Source: Ln %d, Col %d\t\tToken: tk_ELLIPSIS\t\tValue: ...\n", token.err_ln, token.err_col); break;
            case tk_NUM: printf("Source: Ln %d, Col %d\t\tToken: tk_NUM\t\tSemanticValue: %d\n", token.err_ln, token.err_col, token.n); break;
            case tk_STRINGLIT: printf("Source: Ln %d, Col %d\t\tToken: tk_STRINGLIT\t\tSemanticValue: %s\n", token.err_ln, token.err_col, token.text); break;
            case tk_SYM:  printf("Source: Ln %d, Col %d\t\tToken: tk_SYM\t\tSemanticValue: %s\n", token.err_ln, token.err_col, token.text); break;

        }
}

void lex(){
    FILE* outFile;
    fopen_s(&outFile,"Serialize.txt", "a+,ccs=UNICODE");
    Token token;

    struct timeb start, end;
    ftime(&start);

    do{
        token = get_token();
        if (token.tokenType == tk_SYM){
            fwprintf(outFile, L"%hs\n", token.text);
        }
    if (LEXOUTPUT == 1)
        lexPrint(token);
    }while(token.tokenType != tk_EOF);
    ftime(&end);
    int diff = (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm));
   
    printf("\nLexing time: %u milliseconds = %u microseconds", diff, diff*1000);
    fclose(outFile);
    fclose(source_fp);
}

int main(int argc, char *argv[]){
  
    if (argc >= 2){
        LEXOUTPUT  = strtol(argv[2], NULL, 10);
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