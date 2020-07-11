#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

#define NELEMS(arr) (sizeof(arr) / sizeof(arr[0]))

#define da_dim(name, type)  type *name = NULL;          \
                            int _qy_ ## name ## _p = 0;  \
                            int _qy_ ## name ## _max = 0
#define da_rewind(name)     _qy_ ## name ## _p = 0
#define da_redim(name)      do {if (_qy_ ## name ## _p >= _qy_ ## name ## _max) \
                                name = realloc(name, (_qy_ ## name ## _max += 56) * sizeof(name[0]));} while (0)
#define da_append(name, x)  do {da_redim(name); name[_qy_ ## name ## _p++] = x;} while (0)
#define da_len(name)        _qy_ ## name ## _p

typedef enum {
  tk_EOI,
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


//   tk_Mul, tk_Div, tk_Mod, tk_Add, tk_Sub, tk_Negate, tk_Not, tk_Lss, tk_Leq, 
//   tk_Gtr, tk_Geq, tk_Eq, tk_Neq, tk_Assign, tk_And, tk_Or, tk_If, tk_Else, tk_While, 
//   tk_Print, tk_Putc, tk_Lparen, tk_Rparen, tk_Lbrace, tk_Rbrace, tk_Semi, tk_Comma, 
//   tk_Ident, tk_Integer, tk_String
} TokenType;

typedef struct {
 TokenType tok;
 int err_ln, err_col;
 union {
     int n; /* value for constants */
     char* text; /* text for idents */
 };
} tok_s;

static FILE *source_fp, *dest_fp;
static int line =1, col = 0, the_ch = ' ';
da_dim(text, char);

tok_s gettok();

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

static tok_s char_lit(int n, int err_line, int err_col){ /* 'x' */
 if (the_ch == '\'')
    error(err_line, err_col, "gettok: empty character constant");
if (the_ch == '\\'){
    next_ch();
    if(the_ch == 'n')
        n = 10;
    else if (the_ch == '\\')
        n = '\\';
    else error(err_line, err_col, "gettok: unknown escape sequence \\%c", the_ch);
} 
if(next_ch() != '\'')
    error(err_line, err_col, "multi-character constant");
next_ch();
return (tok_s){tk_NUM, err_line, err_col, {n}};
}

static tok_s div_or_cmt(int err_line, int err_col) {  /* (divide)'/' or comment */
    if(the_ch != '*')
        return (tok_s){tk_DIV, err_line, err_col, {0}};
   /* when comment found */
   next_ch();
    for (;;) {
        if (the_ch == '*') {
            if (next_ch() == '/') {
                next_ch();
                return gettok();
            }
        } else if (the_ch == EOF)
            error(err_line, err_col, "EOF in comment");
        else
            next_ch();
    }
}


static tok_s string_lit(int start, int err_line, int err_col){ /* "string literal" */
    da_rewind(text);

    while(next_ch() != start){
        if(the_ch == '\n') error(err_line, err_col, "EOL in string");
        if (the_ch == EOF) error(err_line, err_col, "EOF in string");
        da_append(text, (char)the_ch);
    }
    da_append(text, '\0');

    next_ch();
    return (tok_s){tk_STRINGLIT, err_line, err_col, {.text=text}};
}

static int kwd_cmp(const void *p1, const void *p2){
    return strcmp(*(char **)p1, *(char **)p2);
}

static TokenType get_ident_type(const char *ident){
    static struct{
        char *s;
        TokenType sym;
    } kwds[] = {
        // {"else", tk_Else},
        // {"if", tk_If},
        // {"print", tk_Print},
        // {"putc", tk_Putc},
        // {"while", tk_While},

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
        {"->", tk_ARROW},
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

static tok_s sym_or_num(int err_line, int err_col){
    int n, is_number = true;
    da_rewind(text);
    while(isalnum(the_ch) || the_ch == "_"){
        da_append(text, (char)the_ch);
        if (!isdigit(the_ch))
            is_number = false;
        next_ch();
    }
    if (da_len(text) == 0)
        error(err_line, err_col, "gettok: unrecognized character (%d) '%c'\n", the_ch, the_ch);
    da_append(text, '\0');
    if(isdigit(text[0])){
        if(!is_number)
            error(err_line, err_col, "invalid number: %s\n", text);
        n = strtol(text, NULL, 0);
        if (n == LONG_MAX && errno == ERANGE)
            error(err_line, err_col, "Number exceeds maximum value");
        return (tok_s){tk_NUM, err_line, err_col, {n}};
    }
    return (tok_s) {get_ident_type(text), err_line, err_col, {.text=text}};
}


static tok_s lookahead(int except, TokenType foundnext, TokenType self, int err_line, int err_col){
  
    if (the_ch == except){
         next_ch();
        return (tok_s){foundnext, err_line, err_col, {0}};
    }
    if(self == tk_EOI)
        error(err_line, err_col, "follow: unrecognized character '%c' (%d)\n", the_ch, the_ch);
      
    return (tok_s){self, err_line, err_col, {0}};
}

static tok_s lookahead2(int except1, int except2, TokenType foundl2, TokenType foundl1, TokenType self, int err_line, int err_col){
  
    if (self == tk_EOI)
            error(err_line, err_col, "follow: unrecognized character '%c' (%d)\n", the_ch, the_ch);

    if (the_ch == except1){
        next_ch();
    
        if (the_ch == except2){
            next_ch();
            return (tok_s){foundl2, err_line, err_col, {0}};
        }else{
            if (foundl1 != 0)
                return (tok_s){foundl1, err_line, err_col, {0}};
        }
    }
          
        return (tok_s){self, err_line, err_col, {0}};
}



tok_s gettok(){
    /* skip whitespace */
    while(isspace(the_ch))
        next_ch();
    int err_line = line;
    int err_col = col;
    switch(the_ch){
        case '+': next_ch(); return (tok_s){tk_ADD, err_line, err_col, {0}};
        case '-': next_ch(); return (tok_s){tk_SUB, err_line, err_col, {0}};
        case '*': next_ch(); return (tok_s){tk_MUL, err_line, err_col, {0}};
        case '/': next_ch(); return div_or_cmt(err_line, err_col);
        case '%': next_ch(); return lookahead('=', tk_EQMOD, tk_MOD, err_line, err_col);
        case '&': 
        {   
            next_ch(); 
            tok_s token = lookahead('=', tk_EQAND, tk_AND, err_line, err_col);
            if (token.tok == tk_EQAND)
                return token;
            token = lookahead('&', tk_LOGICAND, tk_AND, err_line, err_col);
            if (token.tok == tk_LOGICAND)
                return token;
            token = lookahead2('^', '=', tk_EQANDXOR, tk_ANDXOR, tk_AND, err_line, err_col);
            if (token.tok == tk_EQANDXOR || token.tok == tk_ANDXOR)
                return token;
            if (token.tok == tk_AND)
                return token;
            
        }
        //   tok_s token = lookahead2('^', '=', tk_EQANDXOR, tk_AND, err_line, err_col);
        //   if (token.tok == tk_AND){
        //         token = lookahead('=', tk_EQAND, tk_AND, err_line, err_col);
        //         if (token.tok == tk_AND){
        //             token = lookahead('&', tk_LOGICAND, tk_AND, err_line, err_col);
        //             if (token.tok == tk_AND){
        //                 return lookahead('^', tk_ANDXOR, tk_AND, err_line, err_col);
        //             }else{
        //                 return token;
        //             }
        //         }else{
        //             return token;
        //         }
        //   }else{
        //       return token;
        //   }
           
        // }

        // case '{': next_ch(); return (tok_s){tk_LCBRACKET, err_line, err_col, {0}};
        // case '}': next_ch(); return (tok_s){tk_RCBRACKET, err_line, err_col, {0}};
        // case '(': next_ch(); return (tok_s){tk_LPAREN, err_line, err_col, {0}};
        // case ')': next_ch(); return (tok_s){tk_RPAREN, err_line, err_col, {0}};
        // case '+': next_ch(); return (tok_s){tk_ADD, err_line, err_col, {0}};
        // case '-': next_ch(); return (tok_s){tk_SUB, err_line, err_col, {0}};
        // case '*': next_ch(); return (tok_s){tk_MUL, err_line, err_col, {0}};
        // case '%': next_ch(); return (tok_s){tk_MOD, err_line, err_col, {0}};
        // case ';': next_ch(); return (tok_s){tk_SEMI, err_line, err_col, {0}};
        // case ',': next_ch(); return (tok_s){tk_COMMA, err_line, err_col, {0}};

        // case '/': next_ch(); return div_or_cmt(err_line, err_col);
        // case '\'': next_ch(); return char_lit(the_ch, err_line, err_col);
        // case '<': next_ch(); return follow('=', tk_Leq, tk_Lss, err_line, err_col);
        // case '>': next_ch(); return follow('=', tk_Geq, tk_Gtr, err_line, err_col);
        // case '=': next_ch(); return follow('=', tk_Eq, tk_Assign, err_line, err_col);
        // case '!': next_ch(); return follow('=', tk_Neq, tk_Not, err_line, err_col);
        // case '&': next_ch(); return follow('&', tk_And, tk_EOI, err_line, err_col);
        // case '|': next_ch(); return follow('|', tk_Or, tk_EOI, err_line, err_col);
        // case '"': return string_lit(the_ch, err_line, err_col);
        default: return sym_or_num(err_line, err_col);
        case EOF: return (tok_s){tk_EOI, err_line, err_col, {0}}; // WHATTTTTTTTTTT????
    }
}

void lex(){
    tok_s tok;
    do{
        tok = gettok();
      
        switch(tok.tok){
            case tk_EOI: printf("%d:%d              tk_EOI\n", tok.err_ln, tok.err_col); break;
            case tk_BREAK: printf("%d:%d            tk_BREAK\n", tok.err_ln, tok.err_ln); break;
            case tk_CASE: printf("%d:%d             tk_CASE\n", tok.err_ln, tok.err_col); break;
            case tk_CHAN: printf("%d:%d             tk_CHAN\n", tok.err_ln, tok.err_col); break;
            case tk_CONST: printf("%d:%d            tk_CONST\n", tok.err_ln, tok.err_col); break;
            case tk_CONTINUE: printf("%d:%d         tk_CONTINUE\n", tok.err_ln, tok.err_col); break;
            case tk_DEFAULT: printf("%d:%d          tk_DEFAULT\n", tok.err_ln, tok.err_col); break;
            case tk_DEFER: printf("%d:%d            tk_DEFER\n", tok.err_ln, tok.err_col); break;
            case tk_ELSE: printf("%d:%d             tk_ELSE\n", tok.err_ln, tok.err_col); break;
            case tk_FALLTHROUGH: printf("%d:%d      tk_FALLTHROUGH\n", tok.err_ln, tok.err_col); break;
            case tk_FOR: printf("%d:%d              tk_FOR\n", tok.err_ln, tok.err_col); break;
            case tk_FUNC: printf("%d:%d             tk_FUNC\n", tok.err_ln, tok.err_col); break;
            case tk_GO: printf("%d:%d               tk_GO\n", tok.err_ln, tok.err_col); break;
            case tk_GOTO: printf("%d:%d             tk_GOTO\n", tok.err_ln, tok.err_col); break;
            case tk_IF: printf("%d:%d               tk_IF\n", tok.err_ln, tok.err_col); break;
            case tk_IMPORT: printf("%d:%d           tk_IMPORT\n", tok.err_ln, tok.err_col); break;
            case tk_INTERFACE: printf("%d:%d        tk_INTERFACE\n", tok.err_ln, tok.err_col); break;
            case tk_MAP: printf("%d:%d              tk_MAP\n", tok.err_ln, tok.err_col); break;
            case tk_PACKAGE: printf("%d:%d          tk_PACKAGE\n", tok.err_ln, tok.err_col); break;
            case tk_RANGE: printf("%d:%d            tk_RANGE\n", tok.err_ln, tok.err_col); break;
            case tk_RETURN: printf("%d:%d           tk_RETURN\n", tok.err_ln, tok.err_col); break;
            case tk_SELECT: printf("%d:%d           tk_SELECT\n", tok.err_ln, tok.err_col); break;
            case tk_STRUCT: printf("%d:%d           tk_STRUCT\n", tok.err_ln, tok.err_col); break;
            case tk_SWITCH: printf("%d:%d           tk_SWITCH\n", tok.err_ln, tok.err_col); break;
            case tk_TYPE: printf("%d:%d             tk_TYPE\n", tok.err_ln, tok.err_col); break;
            case tk_VAR: printf("%d:%d              tk_VAR\n", tok.err_ln, tok.err_col); break;
            case tk_CLASS: printf("%d:%d            tk_CLASS\n", tok.err_ln, tok.err_col); break;
            case tk_EXTENDS: printf("%d:%d          tk_EXTENDS\n", tok.err_ln, tok.err_col); break;
            case tk_IMPLEMENTS: printf("%d:%d       tk_IMPLEMENTS\n", tok.err_ln, tok.err_col); break;
            case tk_NEW: printf("%d:%d              tk_NEW\n", tok.err_ln, tok.err_col); break;
            case tk_SUPER: printf("%d:%d            tk_SUPER\n", tok.err_ln, tok.err_col); break;
            case tk_PUBLIC: printf("%d:%d           tk_PUBLIC\n", tok.err_ln, tok.err_col);  break;
            case tk_PRIVATE: printf("%d:%d          tk_PRIVATE\n", tok.err_ln, tok.err_col); break;
            case tk_INSTANCEOF: printf("%d:%d       tk_INSTANCEOF\n", tok.err_ln, tok.err_col ); break;
            case tk_PTRSELECT: printf("%d:%d        tk_PTRSELECT\n", tok.err_ln, tok.err_col); break;
            case tk_OVERRIDE: printf("%d:%d         tk_OVERRIDE\n", tok.err_ln, tok.err_col); break;
            case tk_T_STRING: printf("%d:%d         tk_T_STRING\n", tok.err_ln, tok.err_col); break;
            case tk_T_BOOL: printf("%d:%d           tk_T_BOOL\n", tok.err_ln, tok.err_col); break;
            case tk_T_INT8: printf("%d:%d           tk_T_INT8\n", tok.err_ln, tok.err_col); break;
            case tk_T_UINT8: printf("%d:%d          tk_T_UINT8\n", tok.err_ln, tok.err_col); break;
            case tk_T_BYTE: printf("%d:%d           tk_T_BYTE\n", tok.err_ln, tok.err_col); break;
            case tk_T_INT16: printf("%d:%d          tk_T_INT16\n", tok.err_ln, tok.err_col); break;
            case tk_T_UINT16: printf("%d:%d         tk_T_UINT16\n", tok.err_ln, tok.err_col); break;
            case tk_T_INT32: printf("%d:%d          tk_T_INT32\n", tok.err_ln, tok.err_col); break;
            case tk_T_UINT32: printf("%d:%d         tk_T_UINT32\n",tok.err_ln, tok.err_col); break;
            case tk_T_RUNE: printf("%d:%d           tk_T_RUNE\n", tok.err_ln, tok.err_col); break;
            case tk_T_INT64: printf("%d:%d          tk_T_INT64\n", tok.err_ln, tok.err_col); break;
            case tk_T_UINT64: printf("%d:%d         tk_T_UINT64\n", tok.err_ln, tok.err_col); break;
            case tk_T_INT: printf("%d:%d            tk_T_INT\n", tok.err_ln, tok.err_col); break;
            case tk_T_UINTPTR: printf("%d:%d        tk_UINTPTR\n", tok.err_ln, tok.err_col); break;
            case tk_T_FLOAT32: printf("%d:%d        tk_T_FLOAT32\n", tok.err_ln, tok.err_col); break;
            case tk_T_FLOAT64: printf("%d:%d        tk_T_FLOAT64\n", tok.err_ln, tok.err_col); break;
            case tk_T_COMPLEX64: printf("%d:%d      tk_T_COMPLEX64\n", tok.err_ln, tok.err_col); break;
            case tk_T_COMPLEX128: printf("%d:%d     tk_T_COMPLEX128\n", tok.err_ln, tok.err_col); break;
            case tk_ADD: printf("%d:%d              tk_ADD\n", tok.err_ln, tok.err_col); break;
            case tk_SUB: printf("%d:%d              tk_SUB\n", tok.err_ln, tok.err_col); break;
            case tk_MUL: printf("%d:%d              tk_MUL\n", tok.err_ln, tok.err_col); break;
            case tk_DIV: printf("%d:%d              tk_DIV\n", tok.err_ln, tok.err_col); break;
            case tk_MOD: printf("%d:%d              tk_MOD\n", tok.err_ln, tok.err_col); break;
            case tk_AND: printf("%d:%d              tk_AND\n", tok.err_ln, tok.err_col); break;
            case tk_OR: printf("%d:%d               tk_OR\n", tok.err_ln, tok.err_col); break;
            case tk_XOR: printf("%d:%d              tk_XOR\n", tok.err_ln, tok.err_col); break;
            case tk_ASSIGN: printf("%d:%d           tk_ASSIGN\n", tok.err_ln, tok.err_col); break;
            case tk_LPAREN: printf("%d:%d           tk_LPAREN\n", tok.err_ln, tok.err_col); break;
            case tk_RPAREN: printf("%d:%d           tk_RPAREN\n", tok.err_ln, tok.err_col); break;
            case tk_LSBRACKET: printf("%d:%d        tk_LSBRACKET\n", tok.err_ln, tok.err_col); break;
            case tk_RSBRACKET: printf("%d:%d        tk_RSBRACKET\n", tok.err_ln, tok.err_col); break;
            case tk_LCBRACKET: printf("%d:%d        tk_LCBRACKET\n", tok.err_ln, tok.err_col); break;
            case tk_RCBRACKET: printf("%d:%d        tk_RCBRACKET\n", tok.err_ln, tok.err_col); break;
            case tk_COMMA: printf("%d:%d            tk_BREAK\n", tok.err_ln, tok.err_col); break;
            case tk_DOT: printf("%d:%d              tk_DOT\n", tok.err_ln, tok.err_col); break;
            case tk_SEMI: printf("%d:%d             tk_SEMI\n", tok.err_ln, tok.err_col); break;
            case tk_UNDERSCORE: printf("%d:%d       tk_UNDERSCORE\n", tok.err_ln, tok.err_col); break;
            case tk_COLON: printf("%d:%d            tk_COLON\n", tok.err_ln, tok.err_col); break;
            case tk_LSHIFT: printf("%d:%d           tk_LSHIFT\n", tok.err_ln, tok.err_col); break;
            case tk_RSHIFT: printf("%d:%d           tk_RSHIFT\n", tok.err_ln, tok.err_col); break;
            case tk_EQXOR: printf("%d:%d            tk_EQXOR\n", tok.err_ln, tok.err_col); break;
            case tk_EQOR: printf("%d:%d             tk_EQOR\n", tok.err_ln, tok.err_col); break;
            case tk_EQAND: printf("%d:%d            tk_EQAND\n", tok.err_ln, tok.err_col); break;
            case tk_EQANDXOR: printf("%d:%d         tk_EQANDXOR\n", tok.err_ln, tok.err_col); break;
            case tk_EQRSHIFT: printf("%d:%d         tk_EQRSHIFT\n", tok.err_ln, tok.err_col); break;
            case tk_LOGICAND: printf("%d:%d         tk_LOGICKAND\n", tok.err_ln, tok.err_col); break;
            case tk_EQLSHIFT: printf("%d:%d         tk_EQLSHIFT\n", tok.err_ln, tok.err_col); break;
            case tk_LOGICOR: printf("%d:%d          tk_LOGICOR\n", tok.err_ln, tok.err_col); break;
            case tk_EQADD: printf("%d:%d            tk_EQADD\n", tok.err_ln, tok.err_col); break;
            case tk_EQSUB: printf("%d:%d            tk_EQSUB\n", tok.err_ln, tok.err_col); break;
            case tk_EQMUL: printf("%d:%d            tk_EQMUL\n", tok.err_ln, tok.err_col); break;
            case tk_EQDIV: printf("%d:%d            tk_EQDIV\n", tok.err_ln, tok.err_col); break;
            case tk_EQMOD: printf("%d:%d            tk_EQMOD\n", tok.err_ln, tok.err_col); break;
            case tk_ANDXOR: printf("%d:%d           tk_ANDXOR\n", tok.err_ln, tok.err_col); break;
            case tk_NEG: printf("%d:%d              tk_NEG\n", tok.err_ln, tok.err_col ); break;
            case tk_LSS: printf("%d:%d              tk_LSS\n", tok.err_ln, tok.err_col); break;
            case tk_GRT: printf("%d:%d              tk_GRT\n", tok.err_ln, tok.err_col); break;
            case tk_NOTEQ: printf("%d:%d            tk_NOTEQ\n", tok.err_ln, tok.err_col); break;
            case tk_EQ: printf("%d:%d               tk_EQ", tok.err_ln, tok.err_col ); break;
            case tk_EQLSS: printf("%d:%d            tk_BREAK\n", tok.err_ln, tok.err_col); break;
            case tk_EQGRT: printf("%d:%d            tk_BREAK\n", tok.err_ln, tok.err_col); break;
            case tk_SHORTDECL: printf("%d:%d        tk_SHORTDECL\n", tok.err_ln, tok.err_col); break;
            case tk_ARROW: printf("%d:%d            tk_ARROW\n", tok.err_ln, tok.err_col); break;
            case tk_INC: printf("%d:%d              tk_INC\n", tok.err_ln, tok.err_col); break;
            case tk_DEC: printf("%d:%d              tk_DEC\n", tok.err_ln, tok.err_col); break;
            case tk_ELLIPSIS: printf("%d:%d         tk_ELLIPSIS\n", tok.err_ln, tok.err_col); break;
            case tk_NUM: printf("%d:%d              tk_NUM      %d\n", tok.err_ln, tok.err_col, tok.n); break;
            case tk_STRINGLIT: printf("%d:%d        tk_STRINGLIT    %s\n", tok.err_ln, tok.err_col, tok.text); break;
            case tk_SYM: printf("%d:%d              tk_SYM      %s\n", tok.err_ln, tok.err_col, tok.text); break;

        }


        // fprintf(dest_fp, "%5d %5d %.15s",
        // tok.err_ln, tok.err_col,
        // &"End_of_input    tk_BREAK        tk_CASE         tk_mod          Op_add          "
        //  "Op_subtract     Op_negate       Op_not          Op_less         Op_lessequal    "
        //  "Op_greater      Op_greaterequal Op_equal        Op_notequal     Op_assign       "
        //  "Op_and          Op_or           Keyword_if      Keyword_else    Keyword_while   "
        //  "Keyword_print   Keyword_putc    LeftParen       RightParen      LeftBrace       "
        //  "RightBrace      Semicolon       Comma           Identifier      Integer         "
        //  "String          "           
        // [tok.tok *16]);
        // if(tok.tok == tk_NUM)   fprintf(dest_fp, "  %4d",   tok.n);
        // else if(tok.tok == tk_SYM) fprintf(dest_fp, " %s", tok.text);
        // else if(tok.tok == tk_STRINGLIT) fprintf(dest_fp, "\"%s\"", tok.text);
        //fprintf(dest_fp, "\n");
    }while(tok.tok != tk_EOI);
    if (dest_fp != stdout)
        fclose(dest_fp);
}

void init_io(FILE **fp, FILE *std, const char mode[], const char fn[]){
    if(fn[0] == '\0')
        *fp = std;
    else if((fopen_s(fp, fn, mode)) == NULL)
        error(0,0, "Can't open %s\n", fn);
}

int main(int argc, char *argv[]){
    init_io(&source_fp, stdin, "r", argc > 1 ? argv[1]: "" );
    init_io(&dest_fp, stdout, "wb", argc > 2 ? argv[2]: "");
    lex();
    return 0;
}




// TODO: tok_s string_lit ...