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
void parse_import_decl_with_alias(char* );
void parse_import_decl_list();
void parse_var_decl_name_list();
void parse_var_decl_expr_list();
void parse_var_decl_expr();
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

int is_rule_for_var = 0;
int in_list = 0;
int is_rule_for_builtint_type = 0;
int is_rule_for_usertype = 0;
int is_rule_for_pointer_type = 0 ;

void rule_top_var_const()
{
    backup_position(); // backup position after tk_VAR/tk_CONST for "expected ident or type name" error
    if (in_list != 1)
        next_token();

    rule_inner_builtin_type();

    
   
    if (_token == tk_IDENT)
    {
        char * semantic_value_var_name_or_usertype = _strdup(_semantic_value);

        next_token();

         // do the same as with tk_MUL upper do for qualifed package and atc

        if (_token == tk_IDENT)
        {
            char* qualified_name_or_type_or_name = _strdup(_semantic_value);
            next_token();

            if (_token == tk_DOT)
            {
                next_token();
                
            }
        }

         // *** save line **  

        if (_token == tk_MUL)
        {
            is_rule_for_pointer_type = 1;
            next_token();
            if (_token == tk_COMMA || _token == tk_ASSIGN || _token == tk_SEMI)
            {
                apxerror_custom_position_fatal(line, col, "pointer can't be here");
            } 
        }

       

        if (_token == tk_IDENT)
        {
            is_rule_for_usertype = 1;
            printf("var: usertype: '%s'", semantic_value_var_name_or_usertype);
            printf("var: name '%s'", _semantic_value);
            next_token();
        }
       
        else 
        {
           
            printf("var: name '%s'", semantic_value_var_name_or_usertype);
        }
        printf("token is %u", _token);


        if (_token == tk_COMMA)
        {
               next_token();
               in_list =1; 
               rule_inner_ident_list();
               in_list =0;
               
        }

        if (_token == tk_ASSIGN)
        {
            next_token();
            rule_inner_initializer();
        }

        
       
    }else{
        apxerror_custom_position_fatal(_backed_line,_backed_col,"expected ident");
    }
    
}

void rule_inner_ident_list()
{
    do{
        rule_top_var_const();
    }while(_token == tk_IDENT || _token == tk_COMMA);
}

void rule_inner_initializer()
{
    if (_token == tk_IDENT || _token == tk_STRINGLIT || _token == tk_NUM)
    {
        printf("var: value '%s' ", _semantic_value);
        next_token();
        if (_token == tk_COMMA)
        {
            next_token();
            rule_inner_initializer_list();
        }
    }
}

void rule_inner_initializer_list()
{
    do{
        rule_inner_initializer();
    }while(_token == tk_IDENT || _token == tk_STRINGLIT || _token == tk_NUM);
}













// void parse_import_decl_list(){
//     do {
//          rule_top_include();
//     }while(_token == tk_IDENT || _token == tk_STRINGLIT || _token == tk_AS);
// }



/* IMPORTS RULE END */
int is_var_decl = 0;

/* VAR RULE BEGIN */
void parse_var_decl_name(){
    if (from_scope != 1) {
        _token = apxlex();
    }

           if(_token == tk_IDENT){
             printf("var: name '%s' ", get_queued_semantic_value());
             _token = apxlex();
           
    
             if (_token == tk_MUL){
                 is_rule_for_pointer_type = 1;
                 _token = apxlex();
                 if (_token != tk_IDENT && is_builtin_type(_token) != 1)
                 {
                     apxerror_fatal("expected type name after pointer symbol '*'");
                 } 
             }
             
             rule_inner_builtin_type();
            /* 
                for const declaration only 
            */
             if (is_var_decl == 0){ 
                 if(_token == tk_IDENT){
                     apxerror_fatal("invalid constant type %s", get_queued_semantic_value());
                 }
             }

            /* 
                for var declaration only 
            */
            if(is_var_decl == 1){  
             if(_token == tk_IDENT){
                 char *semantic_value_qualifiedpackage_or_type = _strdup(get_queued_semantic_value());
                 _token = apxlex();
                 if (_token == tk_DOT){
                     _token = apxlex();
                     if (_token == tk_IDENT){
                         printf("var: type (qualified) '%s' from package '%s' ", get_queued_semantic_value(), semantic_value_qualifiedpackage_or_type);
                         if (is_rule_for_pointer_type == 1)
                            printf("var: type (qualified ptr) '%s' from package '%s' ", get_queued_semantic_value(), semantic_value_qualifiedpackage_or_type);
                          _token = apxlex();
                     }else{
                         apxerror_fatal("expected qualified type after '.'\n");
                     }
                 }else{

                    printf("var: type '%s' ", semantic_value_qualifiedpackage_or_type);
                    if(is_rule_for_pointer_type == 1)
                        printf("var: type (ptr) '%s' ", semantic_value_qualifiedpackage_or_type);
                 }

             }
            }

            if (_token == tk_ASSIGN){
                _token = apxlex();
                parse_var_decl_expr();
                }else{
                    if (is_var_decl == 0){ // if const declaration was/is
                    apxerror("const declaration cannot have type without expression");
                    apxerror_fatal( "missing value in const declaration");

               
            }

            
        }
           
             if (_token == tk_COMMA){
                _token = apxlex();
                from_scope = 1;
                parse_var_decl_name_list();
                if (_token == tk_ASSIGN){
                    _token = apxlex();
                    parse_var_decl_expr();
                    

                }else{
                    if (is_var_decl == 0){ // if const declaration was/is
                        apxerror("const declaration cannot have type without expression");
                        apxerror_fatal("missing value in const declaration");

               // apxerror("var declaration cannot have type without expression\n");
                }

            
        }
                // parse_var_decl_expr();
                // HERE ??? 
               
                from_scope = 0; // or before parse_var_decl_expr(); ??? line up
            } 
        }

   // }  end 

}


void parse_var_decl_expr(){
    switch(_token){
        case tk_IDENT:
        case tk_STRINGLIT:
        case tk_NUM:
            printf("var: value '%s'", get_queued_semantic_value());
            _token = apxlex();
            if (_token == tk_COMMA){
                _token = apxlex();
                // from_scope =1 ??
                parse_var_decl_expr_list();
                // from_scope = 0;
            }
    }
}

void parse_var_decl_expr_list(){
    do{
        parse_var_decl_expr();
    }while(_token == tk_IDENT || _token == tk_STRINGLIT || _token == tk_NUM || _token == tk_COMMA);
}

void parse_var_decl_name_list(){
    do{
        parse_var_decl_name();
    }while(_token == tk_IDENT || _token == tk_COMMA);
   // is_var_name_list_end = 1;
}

void parse_top_vars_or_const_decl(){
    parse_var_decl_name();
    

    // if (_token == tk_ASSIGN){
    //     _token = apxlex();
    //     parse_var_decl_expr();
    //    // parse_grammar();
    //     }else{
    //         if (is_var_decl == 0){ // if const declaration was/is
    //             apxerror("const declaration cannot have type without expression");
    //             apxerror_fatal( "missing value in const declaration");

    //            // apxerror("var declaration cannot have type without expression\n");
    //         }

            
    //     }
}
/* VAR RULE END */

/*SPECIAL RULE BUILTIN TYPE BEGIN*/
void rule_inner_builtin_type(){
    switch(_token){
        case tk_T_BOOL:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'bool'");    
            printf("var: type (builtin) 'bool'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_BYTE:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'byte'");
            printf("var: type (builtin) 'byte'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_COMPLEX128:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'complex128'");
            printf("var: type (builtin) 'complex128'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_COMPLEX64:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'complex64'");
            printf("var: type (builtin) 'complex64'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_FLOAT32:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'float32'");
            printf("var type (builtin) 'float32'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_INT16:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'int16'");
            printf("var: type (builtin) 'int16'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_INT32:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'int32'");
            printf("var: type (builtin) 'int32'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_INT64:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'int64'");
            printf("var: type (builtin) 'int64'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_INT8:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'int8'");
            printf("var: type (builtin) 'int8'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_INT:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'int'");
            printf("var: type (builtin) 'int'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_RUNE:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'rune'");
            printf("var: type (builtin) 'rune'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_STRING:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'string'");
            printf("var: type (builtin) 'string'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_UINT16:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint16'");
            printf("var: type (builtin) 'uint16'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_UINT32:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint32'");
            printf("var: type (builtin) 'uint32'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_UINT64:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint64'");
            printf("var: type (builtin) 'uint64'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_UINT8:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint8'");
            printf("var: type (builtin) 'uint8'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_UINT:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'uint'");
            printf("var: type (builtin) 'uint'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
            break;
        case tk_T_UINTPTR:
            if(is_rule_for_pointer_type == 1)
                printf("var: type (builtin ptr) 'uintptr'");
            printf("var: type (builtin) 'uintptr'");
            backup_position();
            is_rule_for_builtint_type = 1;
            _token = apxlex();
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
        /* TEST RULE; BEGIN THIS */
        // case tk_CLASS:
        //     printf("hello class\n");
        //     _token = apxlex();
        //     parse_grammar();
        //     break;
        case tk_VAR:
            is_rule_for_var = 1;
            rule_top_var_const();
            rule_special_terminator();
            parse_grammar();
            break;
            // is_var_decl = 1;
            // parse_top_vars_or_const_decl();
            // parse_grammar();
           // break;
        case tk_CONST:
            is_rule_for_var = 0;
            rule_top_var_const();
            parse_grammar();
            break;
            // is_var_decl = 0;
            // parse_top_vars_or_const_decl();
            // parse_grammar();
            // break;
        case tk_IDENT:
        case tk_NUM:
        case tk_STRINGLIT:
            apxerror_fatal("unexpected '%s', expected declaration or statement", get_queued_semantic_value());
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