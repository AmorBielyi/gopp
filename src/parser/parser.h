typedef enum {
    nd_IDENT, nd_STRING, nd_INTEGER, nd_SEQUENCE, nd_IF, nd_PRTC, nd_PRTS, nd_PRTI, nd_WHILE,
    nd_ASSIGN, nd_NEGATE, nd_NOT, nd_MUL, nd_DIV, nd_MOD, nd_ADD, nd_SUB, nd_LSS, nd_LEQ,
    nd_GTR, nd_GEQ, nd_EQL, nd_NEQ, nd_AND, nd_OR
} NodeType;

typedef struct Tree {
    NodeType node_type;
    struct Tree *left;
    struct Tree *right;
    char *value;
}Tree;

Tree *parse();
void prt_ast(Tree *t);