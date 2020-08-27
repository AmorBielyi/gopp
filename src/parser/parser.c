/*
     Apex recursive descent parser in pure C
*/
#include <stdio.h> // for printf
#include <string.h> // for _strdup
#include "../lexer/lexer.h"


/*
    globals
*/
static token_type _token;
static char *_semantic_value;
static int _backed_line, _backed_col;
extern int line, col;


/*
    interfaces
*/

void rule_inner_builtin_type();
void rule_inner_ident_list();
void rule_inner_initializer_list();
void rule_inner_initializer();
int is_builtin_type(token_type _token);


/*
    get next token
*/
void next_token()
{
    _token = apxlex();
    _semantic_value = get_queued_semantic_value();
}

/*
    backup line and column position after specific token
*/
void backup_position()
{
    _backed_line = line;
    _backed_col = col;
}

/*
    RULE NAME: terminator 
    RULE INFO: special rule for terminator 
*/
void rule_special_terminator(){
   
    if (_token == tk_SEMI)
        next_token();
    else 
        apxerror_custom_position_fatal(_backed_line, _backed_col, "expected ';'");
}

/* 
    RULE NAME: package
    RULE INFO: top rule for package   
*/
void rule_top_package()
{   
    
    backup_position(); //backup position after tk_PACKAGE for "expected package name" error

    next_token();
    if(_token == tk_IDENT)
    {
        
        backup_position(); // backup position after package name for "expected ';'" error
        
        printf("package: name '%s'\n", _semantic_value);
        
        next_token();
    }  
    else
        apxerror_custom_position_fatal(_backed_line, _backed_col, "expected identifier");
}


int from_scope = 0;



/* 
    RULE NAME: include/goinclude

    RULE INFO: top rule for with/without alias include and goinclude   
 */
void rule_top_include()
{
    backup_position(); // backup position after tk_INCLUDE/tk_GOINCLUDE for "expected package to include" error
    next_token();
    /*
        sub-rule for package 
    */
    if(_token == tk_STRINGLIT)
    {
        printf("include: package '%s'", _semantic_value);
        next_token();
        /*
            sub-rule for alias  
        */
        if (_token == tk_AS)
        {
            backup_position(); // backup position after tk_AS for "expected alias for package" error
            next_token();
            if (_token == tk_IDENT)
            {
                backup_position(); // backup position after tk_AS for "expected ';'" error
                printf(", alias '%s'", _semantic_value);
                next_token();
            }
            else 
                apxerror_custom_position_fatal(_backed_line, _backed_col,"expected identifier");
        }
        printf("\n"); // for pretty parse result output
        
    }
    else
        apxerror_custom_position_fatal(_backed_line, _backed_col,"expected string literal");

    }

/*
    RULE NAME: var/const
 
    RULE INFO: top rule for is/not qualified/pointer user/builtin type variable/constant
*/

int is_rule_for_var = 0;
int in_list = 0;
int is_rule_for_builtint_type = 0;
int is_rule_for_pointer_type = 0 ;
int is_rule_for_qualified_type = 0;
char *builtin_type_pseudosemantic;

void rule_top_var_const()
{
    backup_position(); // backup position after tk_VAR/tk_CONST for "expected ident or type name" error
    if (in_list != 1)
        next_token();
   
   rule_inner_builtin_type();

   if(builtin_type_pseudosemantic)
   {

       // code here for builtin

       if(is_rule_for_pointer_type == 1)
       {
            if (is_rule_for_var == 1)
                printf("var: type (builtin ptr) '%s'", builtin_type_pseudosemantic);
            if (is_rule_for_var == 0) 
                printf("const: type (builtin ptr) '%s'", builtin_type_pseudosemantic);
       }
        if (in_list == 0 && is_rule_for_var ==  1) 
            printf("var: type (builtin) '%s'", builtin_type_pseudosemantic);
        if (in_list == 0 && is_rule_for_var ==  0)  
            printf("const: type (builtin) '%s'", builtin_type_pseudosemantic);
   }


    if (is_rule_for_builtint_type == 1)
    {
        if (_token == tk_MUL)
        {
            is_rule_for_pointer_type = 1;
            next_token();
        } // maybe remove
    }

    if (_token == tk_IDENT)
    {
        char * backed_semantic = _strdup(_semantic_value);
        backup_position();
        next_token();
        if (is_rule_for_builtint_type !=1)
        {
            if (_token == tk_DOT)
            {
                next_token();
                backup_position();
                if (_token == tk_IDENT)
                {
                    is_rule_for_qualified_type = 1;
                    if (is_rule_for_var == 1)
                        printf("var: usertype (qualified) '%s', package '%s'", _semantic_value, backed_semantic);
                    if (is_rule_for_var == 0) 
                        printf("const: usertype (qualified) '%s', package '%s'", _semantic_value, backed_semantic);
                    next_token();
                }
                 else 
                    apxerror_custom_position_fatal(_backed_line, _backed_col, "expected ident");
            }

            if (_token == tk_MUL)
            {
                is_rule_for_pointer_type = 1;
                next_token();
            }

            if (_token == tk_IDENT)
            {
                if(is_rule_for_qualified_type != 1)
                {
                    if (is_rule_for_var == 1)
                        printf("var: usertype '%s' var: name '%s'",backed_semantic, _semantic_value);
                    if (is_rule_for_var == 0)
                        printf("const: usertype '%s' const: name '%s'", backed_semantic, _semantic_value);
                   // printf(" var: name '%s'", _semantic_value );
                }
                next_token();
            }
            else 
            {
                if (is_rule_for_qualified_type == 1 && in_list != 1)
                    apxerror_custom_position_fatal(line, col, "expected ident");
                if (is_rule_for_var == 1)
                    printf("var: name '%s'", _semantic_value);
                if (is_rule_for_var == 0) 
                    printf("const: name '%s'", _semantic_value);
            }
        }
        if (is_rule_for_builtint_type == 1)
        {
            if (is_rule_for_var == 1)
                printf("var: name '%s'", backed_semantic);
            if (is_rule_for_var == 0)
                printf("const: name '%s'", backed_semantic);
        }
            
    }
    else
        apxerror_custom_position_fatal(_backed_line, _backed_col,"expected ident");
    
        backup_position();

        if (_token == tk_COMMA)
        {
               next_token();
               in_list =1; 
               rule_inner_ident_list();
               in_list =0;
        }
    
        if (_token == tk_ASSIGN)
        {
            backup_position();
            next_token();
            rule_inner_initializer();
        }

        is_rule_for_qualified_type = 0;

}

void rule_inner_ident_list()
{
   // printf("token in rule_inner_ident_list %u", _token);
     if ( _token != tk_IDENT)
        apxerror_custom_position_fatal(_backed_line, _backed_col,"expected ident in list");

    do{
        rule_top_var_const();
    }while(_token == tk_IDENT || _token == tk_COMMA);
  
}

void rule_inner_initializer()
{
    if (_token == tk_IDENT || _token == tk_STRINGLIT || _token == tk_NUM)
    {
        if (is_rule_for_var == 1)
            printf("var: value '%s' ", _semantic_value);
        if (is_rule_for_var == 0) 
            printf("const: value '%s'", _semantic_value);
        next_token();
        backup_position();
        if (_token == tk_COMMA)
        {
            next_token();
            rule_inner_initializer_list();
        }
    }
}

void rule_inner_initializer_list()
{
    if ( _token != tk_NUM && _token != tk_STRINGLIT)
        apxerror_custom_position_fatal(_backed_line, _backed_col,"expected value in list");
    do{
        rule_inner_initializer();
    }while(_token == tk_IDENT || _token == tk_STRINGLIT || _token == tk_NUM);
}


/* 
    RULE NAME: class
    RULE INFO: top rule for class   
*/
int modifier_type = 0;
enum access_modifier_internal_type
{
    ACCESS_MODIFIER_DEFAULT = -1,
    ACCESS_MODIFIER_PRIVATE = 1 ,
    ACCESS_MODIFIER_PUBLIC = 2 
};
void rule_top_class()
{
    
    backup_position();
    // for default type 
    if (modifier_type == ACCESS_MODIFIER_DEFAULT)
    {
        printf("class: modifier = default ");
    }
            
    next_token();
    // for private and public class 
    if (modifier_type == ACCESS_MODIFIER_PRIVATE || 
     modifier_type == ACCESS_MODIFIER_PUBLIC)
    {
        if (_token == tk_CLASS)
        {   
       
        
            if (modifier_type == ACCESS_MODIFIER_PRIVATE)
                printf("class: modifier = private ");
            if (modifier_type == ACCESS_MODIFIER_PUBLIC)
                printf("class: modifier = public ");
            backup_position();
            next_token();
        }
        else 
            apxerror_custom_position_fatal(_backed_line, _backed_col, "expected 'class' ");
    }

    if (_token == tk_IDENT)
    {
        printf("name: '%s'", _semantic_value);
        backup_position();
        next_token();
    }
    else 
        apxerror_custom_position_fatal(_backed_line, _backed_col, "expected ident");
    if (_token == tk_LCBRACKET)
    {
        backup_position();
        next_token();
        /*
            the rest of class' logick will be here (body of the class with members, methods and etc.)
            this gramamr is alive after '{' before '}'
        */
       // the body is ended; check '}' to close body
        if (_token == tk_RCBRACKET)
        {
            backup_position();
            next_token();
            // .. goes as ending to terminator 
        }
        else
            apxerror_custom_position_fatal(_backed_line, _backed_col, "expected '}'");
    }
    else 
        apxerror_custom_position_fatal(_backed_line, _backed_col, "expected '{'");
}

/*SPECIAL RULE BUILTIN TYPE BEGIN*/
void rule_inner_builtin_type() 
{
    switch(_token) {
        case tk_T_BOOL:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_BYTE:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_COMPLEX128:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_COMPLEX64:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_FLOAT32:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_INT16:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_INT32:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_INT64:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_INT8:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_INT:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_RUNE:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_STRING:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_UINT16:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_UINT32:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_UINT64:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_UINT8:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_UINT:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
        case tk_T_UINTPTR:
            is_rule_for_builtint_type = 1;
            builtin_type_pseudosemantic = _strdup(_semantic_value);
            backup_position();
            next_token();
            break;
    } 
}

int is_builtin_type(token_type tok)
{
    switch(tok){
        case tk_T_BOOL:
            return 1;
        case tk_T_BYTE:
            return 1;
        case tk_T_COMPLEX128:
            return 1;
        case tk_T_COMPLEX64:
            return 1;
        case tk_T_FLOAT32:
            return 1;
        case tk_T_FLOAT64:
            return 1;
        case tk_T_INT16:
            return 1;
        case tk_T_INT32:
            return 1;
        case tk_T_INT64:
            return 1;
        case tk_T_INT8:
            return 1;
        case tk_T_INT:
            return 1;
        case tk_T_RUNE:
            return 1;
        case tk_T_STRING:
            return 1;
        case tk_T_UINT16:
            return 1;
        case tk_T_UINT32:
            return 1;
        case tk_T_UINT64:
            return 1;
        case tk_T_UINT8:
            return 1;
        case tk_T_UINT:
            return 1;
        case tk_T_UINTPTR:
            return 1;
        default:
            return 0;
    }
}


void reset_states_var_const()
{
    is_rule_for_builtint_type = 0;
    is_rule_for_pointer_type = 0;
    is_rule_for_qualified_type = 0;
    is_rule_for_var = 0;
}

/*SPECIAL RULE BUILTIN TYPE END*/
char* decl_type;

void parse_grammar()
{
    switch(_token){
        case tk_PACKAGE:
            rule_top_package();
            rule_special_terminator();
            parse_grammar();
            break;
        case tk_INCLUDE:
        case tk_GOINCLUDE:
            rule_top_include();
            rule_special_terminator();
            parse_grammar();
            break;
        case tk_VAR:
            is_rule_for_var = 1;
            rule_top_var_const();
            rule_special_terminator();
            reset_states_var_const();
            parse_grammar();
            break;
        case tk_CONST:
            rule_top_var_const();
            rule_special_terminator();
            reset_states_var_const();
            parse_grammar();
            break;
        case tk_CLASS:
            modifier_type = ACCESS_MODIFIER_DEFAULT;
            rule_top_class();
            rule_special_terminator();
            modifier_type = 0;
            parse_grammar();
            break;
        case tk_PRIVATE:
            modifier_type = ACCESS_MODIFIER_PRIVATE;
            rule_top_class();
            rule_special_terminator();
            modifier_type = 0;
            parse_grammar();
            break;
        case tk_PUBLIC:
            modifier_type = ACCESS_MODIFIER_PUBLIC;
            rule_top_class();
            rule_special_terminator();
            modifier_type = 0;
            parse_grammar();
            break;
        case tk_IDENT:
        case tk_NUM:
        case tk_STRINGLIT:
            apxerror_custom_position_fatal(line, col,"expected declaration or statement");
    }
}


void gppparse()
{
    _token = apxlex();
    if(_token != tk_PACKAGE)
        apxerror_fatal("expected package statement\n");
    do{
        parse_grammar();
    }while(_token != tk_EOF);
}