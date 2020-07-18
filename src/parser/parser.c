#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>

#include "../lexer/lexer.h"

#define NELEMS(arr) (sizeof(arr)/sizeof(arr[0]))

// typedef enum {
//     tk_EOI, tk_MUL, tk_DIV, tk_MOD, tk_ADD, tk_SUB, tk_NEGATE, tk_NOT, tk_LSS, tk_LEQ, tk_GTR,
//     tk_GEQ, tk_EQL, tk_NEQ, tk_ASSIGN, tk_AND, tk_OR, tk_IF, tk_ELSE, tk_WHILE, tk_PRINT,
//     tk_PUTC, tk_LPAREN, tk_RPAREN, tk_LBRACE, tk_RBRACE, tk_SEMI, tk_COMMA, tk_IDENT,
//     tk_INTEGER, tk_STRING
// } TokenType;

typedef enum {
    nd_IDENT, nd_STRING, nd_INTEGER, nd_SEQUENCE, nd_IF, nd_PRTC, nd_PRTS, nd_PRTI, nd_WHILE,
    nd_ASSIGN, nd_NEGATE, nd_NOT, nd_MUL, nd_DIV, nd_MOD, nd_ADD, nd_SUB, nd_LSS, nd_LEQ,
    nd_GTR, nd_GEQ, nd_EQL, nd_NEQ, nd_AND, nd_OR
} NodeType;

// typedef struct {
//     TokenType token;
//     int err_ln;
//     int err_col;
//     char *text;
// }Token;

typedef struct Tree {
    NodeType node_type;
    struct Tree *left;
    struct Tree *right;
    char *value;
}Tree;
 
struct {
    char *text, *enum_text;
    enum TokenType token;
    bool right_associative, is_binary, is_unary;
    int precedence;
    NodeType node_type;
}atr[] = {
    {"EOF", "End of input", tk_EOF, false ,false ,false, -1, -1},
    {"class", "Keyword class", tk_CLASS, false, false, false, -1,nd_CLASS},
    {"extends", "Keyword extends", tk_EXTENDS, false, false, false, -1, nd_EXTENDS},
    /* Original keywords */
    // {"break", "Keyword break", tk_BREAK, false, false, false,-1,-1},
    // {"case", "Keyword case", tk_CASE, false ,false, false, -1, -1}, // hasn't node type becouse is internal node of switch
    // {"chan", "Keyword chan", tk_CHAN, false ,false, false, -1, nd_CHAN}, // has node type becouse chan followed by type of builtin type
    // {"const", "Keyword const", tk_CONST, false, false ,false, -1, nd_CONST}, // has node type becouse const followed by built-in type
    // {"continue", "Keyword continue", tk_CONTINUE, false, false, false, -1, -1},
    // {"default", "Keyword default", tk_DEFAULT, false, false, false, -1,-1}, //hasn't node type becouse is internal node of switch 
    // {"defer", "Keyword defer", tk_DEFER, false, false, false, -1, nd_DEFER},  //has node type becouse chan followed by function/method call
    // {"else", "Keyword else", tk_ELSE, false, false, false, -1, -1}, //hasn't node type becouse is internal node of if
    // {"fallthrough", "Keyword fallthrough", tk_FALLTHROUGH, false, false, false, -1, -1},
    // {"for", "Keyword for", tk_FOR, false, false, false, -1, nd_FOR},
    // {"go", "Keyword go",tk_GO, false, false, false, -1, nd_GO},
    // {"goto", "Keyword goto", tk_GOTO, false ,false, false, -1, nd_GOTO},
    // {"if", "Keyword if", tk_IF, false,  false , false, -1, nd_IF},
    // {"interface", "Keyword interface", false, false, false, -1, nd_INTERFACE},
    // {"map", "Keyword map", tk_MAP, false, false, false, -1,  nd_MAP},
    // {"package", "Keyword package", tk_PACKAGE, false, false, false, -1, nd_PACKAGE},
    // {"range", "Keyword range", tk_RANGE, false, false, false, -1, nd_RANGE},
    // {"return", "Keyword return", tk_RETURN, false, false, false, -1, nd_RETURN},
    // {"select", "Keyword select", tk_SELECT, false, false, false, -1, nd_SELECT},
    // {"struct", "Keyword struct", tk_STRUCT, false, false, false, -1, nd_STRUCT},
    // {"switch", "Keyword switch", tk_SWITCH, false, false ,false , -1, nd_SWITCH},
    // {"type", "Keyword type", tk_TYPE, false, false, false, -1, nd_TYPE},
    // {"var", "Keyword var", tk_VAR, false, false, false, -1, nd_VAR},
    // /*MilkyWay - original keywords*/
    // {"class", "Keyword class", tk_CLASS, false, false, false, -1, nd_CLASS},
    // {"extends", "Keyword extends", tk_EXTENDS, false, false, false, -1, nd_EXTENDS},
    // {"implements", "Keyword implements", tk_IMPLEMENTS, false, false, false, -1, nd_IMPLEMENTS},
    // {"this", "Keyword this", tk_THIS, false, false, false, -1, nd_THIS},
    // {"public", "Keyword public", tk_PUBLIC, false, false, false, -1, nd_PUBLIC},
    // {"private", "Keyword private", tk_PRIVATE, false, false, false, -1, nd_PRIVATE},
    // {"->", "Op ->", tk_PTRSELECT, false, true, false, -1, nd_PTRSELECT},
    // {"override", "Keyword override", tk_OVERRIDE, false, false, false, -1, nd_OVERRIDE},
    // {"void", "Keyword void", tk_}




    // {"EOI", "End_of_input", tk_EOI, false, false, false, -1, -1},
    // {"*", "Op_multiply", tk_MUL, false, true, false, 13, nd_MUL},
    // {"/", "Op_divide", tk_DIV, false, true, false, 13, nd_DIV},
    // {"%", "Op_mod", tk_MOD, false, true, false, 13, nd_MOD},
    // {"+", "Op_add", tk_ADD, false, true, false, 12, nd_ADD},
    // {"-", "Op_substract", tk_SUB, false, true, false,12,nd_SUB},
    // {"-", "Op_negate", tk_NEGATE, false, false, true, 14, nd_NEGATE},
    // {"!", "Op_not", tk_NOT, false, false, true,14, nd_NOT},
    // {"<", "Op_less", tk_LSS, false, true, false, 10, nd_LSS},
    // {"<=", "Op_lessequal", tk_LEQ, false, true, false, 10, nd_LEQ},
    // {">", "Op_greater", tk_GTR, false, true, false, 10, nd_GTR},
    // {">=", "Op_greaterequal", tk_GEQ, false, true, false, 10, nd_GEQ},
    // {"==", "Op_equal", tk_EQL, false, true, false, 9, nd_EQL},
    // {"!=", "Op_notequal", tk_NEQ, false, true, false ,9, nd_NEQ},
    // {"=", "Op_assign", tk_ASSIGN, false, true, false, -1, nd_ASSIGN},
    // {"&&", "Op_and", tk_AND, false, true, false, 5, nd_AND},
    // {"||", "Op_or", tk_OR, false, true, false, 4, nd_OR},
    // {"if", "Keyword_if", tk_IF, false, false, false, -1, nd_IF},
    // {"else", "Keyword_else", tk_ELSE, false, false, false, -1, -1},
    // {"while", "Keyword_while", tk_WHILE, false, false, false, -1, nd_WHILE},
    // {"print", "Keyword_print", tk_PRINT, false, false, false, -1, -1},
    // {"putc", "Keyword_putc", tk_PUTC, false, false ,false, -1,-1},
    // {"(", "LeftParen", tk_LPAREN, false, false, false, -1,-1},
    // {")", "RightParen", tk_RPAREN, false ,false ,false ,-1, -1},
    // {"{", "LeftBrace", tk_LBRACE, false ,false ,false ,-1, -1},
    // {"}", "RightBrace", tk_RBRACE, false, false ,false ,-1,-1},
    // {";", "Semicolon", tk_SEMI, false, false, false, -1, -1},
    // {",", "Comma", tk_COMMA, false ,false ,false ,-1, -1},
    // {"Ident", "Identifier", tk_IDENT, false ,false ,false , -1, nd_IDENT},
    // {"Integer literal", "Integer", tk_INTEGER, false ,false ,false , -1, nd_INTEGER},
    // {"String literal", "String", tk_STRING, false ,false ,false, -1, nd_STRING},
};

char *Display_nodes[] = {"Identifier", "String", "Integer", "Sequence", "If", "Prtc",
    "Prts", "Prti", "While", "Assign", "Negate", "Not", "Multiply", "Divide", "Mod",
    "Add", "Substract", "Less", "LessEqual", "Greater", "GreaterEqual", "Equal", 
    "NotEqual", "And", "Or"
};
static Token token;
//static FILE *source_fp, *dest_fp;

Tree *paren_expr();

void error(int err_line, int err_col, const char *fmt, ...){
    va_list ap;
    char buf[1000];
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printf("(%d, %d) error: %s\n", err_line, err_col, buf);
    exit(1);
}

// char *read_line(int *len){
//     static char *text = NULL;
//     static int textmax = 0;
//     for(*len = 0; ; (*len)++){
//         int ch = fgetc(source_fp);
//         if (ch == EOF || ch == '\n'){
//             if (*len == 0)
//                 return NULL;
//             break;
//         }
//         if (*len +1 >= textmax){
//             textmax = (textmax == 0?128:textmax*2);
//             text = realloc(text, textmax);
//         }
//         text[*len] = ch;
//     }
//     text[*len] = '\0';
//     return text;
// }

// char *rtrim(char *text, int *len){
//     for (; *len > 0 && isspace(text[*len-1]); --(*len))
//         ;
//     text[*len] = '\0';
//     return text;
// }

TokenType get_enum(const char *name){
    for(size_t i = 0; i < NELEMS(atr); i++){
        if(strcmp(atr[i].enum_text, name) == 0)
            return atr[i].tokenType;
    }
    error(0,0, "Unknown token %s\n", name);
    return 0;
}

// Token get_token(){
//     int len;
//     Token token;
//     char *yytext = read_line(&len); // get from RAM string; that returned by lexer and parse it like just from file
//     yytext = rtrim(yytext, &len);

//     token.err_ln = atoi(strtok(yytext, " "));
//     token.err_col = atoi(strtok(NULL, " "));
//     // get the token name 
//     char *name = strtok(NULL, " ");
//     token.tokenType = get_enum(name);
//     // if there is extra data, get it
//     char *p = name + strlen(name);
//     if(p!= &yytext[len]){
//         for(++p; isspace(*p); ++p)
//             ;
//         token.text = strdup(p);
//     }
//     return token;

// }

Tree *make_node(NodeType node_type, Tree *left, Tree *right){
    Tree *t = calloc(sizeof(Tree),1);
    t->node_type = node_type;
    t->left = left;
    t->right = right;
    return t;
}

Tree *make_leaf(NodeType node_type, char *value){
    Tree *t = calloc(sizeof(Tree), 1);
    t->node_type = node_type;
    t->value = strdup(value);
    return t;
}

void expect(const char msg[], TokenType s){
    if (token.tokenType == s){
        token = get_token();
        return;
    }
    error(token.err_ln, token.err_col,  "%s: Expecting '%s', found '%s'\n", msg, atr[s].text, atr[token.tokenType].text);
}

Tree *expr(int p){
    Tree *x = NULL, *node;
    TokenType op;
    switch(token.tokenType){
        case tk_LPAREN:
            x = paren_expr();
            break;
        case tk_SUB: case tk_ADD:
            op = token.tokenType;
            token = get_token();
            node = expr(atr[tk_NEGATE].precedence);
            x = (op == tk_SUB) ? make_node(nd_NEGATE, node, NULL): node;
            break;
        case tk_NOT:
            token = get_token();
            x = make_node(nd_NOT, expr(atr[tk_NOT].precedence), NULL);
            break;
        case tk_IDENT:
            x = make_leaf(nd_IDENT, token.text);
            token = get_token(); // next_char equivalent
            break;
        default:
            error(token.err_ln, token.err_col, "Expecting a primary, found %s\n", atr[token.tokenType].text);

    }
    while(atr[token.tokenType].is_binary && atr[token.tokenType].precedence >=p){
        TokenType op = token.tokenType;
        token = get_token();
        int q = atr[op].precedence;
        if(!atr[op].right_associative)
            q++;
        node = expr(q);
        x = make_node(atr[op].node_type, x, node);
    }
    return x;
}

Tree *paren_expr(){
    expect("paren_expr", tk_LPAREN);
    Tree *t = expr(0);
    expect("paren_expr", tk_RPAREN);
    return t;
}

Tree *stmt(){
    Tree *t = NULL, *v, *e, *s, *s2;
    switch(token.tokenType){
        case tk_IF:
            token = get_token();
            e = paren_expr();
            s = stmt();
            s2 = NULL;
            if (token.tokenType == tk_ELSE){
                token = get_token();
                s2 = stmt();
            }
            // in future if else was required grammar keyword we will be return error here, that eles expected after if statement; i think i can do it with improved 'expected' function
            t = make_node(nd_IF, e, make_node(nd_IF, s, s2));
            break;
        case tk_PUTC:
            token = get_token();
            e = paren_expr();
            t = make_node(nd_PRTC, e, NULL);
            expect("Putc", tk_SEMI);
            break;
        case tk_PRINT:
            token = get_token();
            for(expect("Print", tk_LPAREN); ; expect("Print", tk_COMMA)){
                if(token.tokenType == tk_STRING){
                    e = make_node(nd_PRTS, make_leaf(nd_STRING, token.text), NULL);
                    token = get_token();
                }else
                    e = make_node(nd_PRTI, expr(0), NULL);

                t = make_node(nd_SEQUENCE, t, e);
                if (token.tokenType != tk_COMMA)
                    break;
            }
            expect("Print", tk_RPAREN);
            expect("Print", tk_SEMI);
            break;
        case tk_SEMI:
            token = get_token();
            break;
        case tk_IDENT:
            v = make_leaf(nd_IDENT, token.text);
            token = get_token();
            expect("assign", tk_ASSIGN);
            e = expr(0);
            t = make_node(nd_ASSIGN, v, e);
            expect("assign", tk_SEMI);
            break;
        case tk_WHILE:
            token = get_token();
            e = paren_expr();
            s = stmt();
            t = make_node(nd_WHILE, e, s);
            break;
        case tk_LBRACE:
            for (expect("Lbrace", tk_LBRACE); token.tokenType != tk_RBRACE && token.tokenType != tk_EOI;)
                t = make_node(nd_SEQUENCE, t, stmt());
            expect("Lbrace", tk_RBRACE);
            break; 
        case tk_EOI:
            break;
        default: error(token.err_ln, token.err_col, "expecting start of statement, found '%s'\n", atr[token.tokenType].text);
    }
    return t;
}

Tree *parse(){
    Tree *t = NULL;
    token =get_token();
    do{
        t = make_node(nd_SEQUENCE, t, stmt());
    }while(t != NULL && token.tokenType != tk_EOI);
    return t;
}

void prt_ast(Tree *t){
    if (t == NULL)
        printf(";\n");
    else {
        printf("%-14s", Display_nodes[t->node_type]);
        if (t->node_type == nd_IDENT || t->node_type == nd_INTEGER || t->node_type == nd_STRING){
            printf("%s\n", t->value);
        }else{
            printf("\n");
            prt_ast(t->left);
            prt_ast(t->right);
        }
    }
}