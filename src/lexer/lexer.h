#include <stdio.h>

long TOKENSDUMP;

static int line =1, col = 0, the_ch = ' ';

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
  tk_SHORTDECL, tk_ARROW, tk_INC, tk_DEC, tk_ELLIPSIS, tk_STRINGLIT, tk_NUM, tk_IDENT, tk_TRUE, tk_FALSE

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

Token get_token();

static void lexerror(int err_line, int err_col, const char* fmt, ...);
void lex(); // temp - not for parser in future