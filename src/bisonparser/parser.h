/* A Bison parser, made by GNU Bison 3.5.0.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_SRC_BISONPARSER_PARSER_H_INCLUDED
# define YY_YY_SRC_BISONPARSER_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    tk_BREAK = 258,
    tk_CASE = 259,
    tk_CHAN = 260,
    tk_CONST = 261,
    tk_CONTINUE = 262,
    tk_DEFAULT = 263,
    tk_DEFER = 264,
    tk_ELSE = 265,
    tk_FALLTHROUGH = 266,
    tk_FOR = 267,
    tk_GO = 268,
    tk_GOTO = 269,
    tk_IF = 270,
    tk_IMPORT = 271,
    tk_INTERFACE = 272,
    tk_MAP = 273,
    tk_PACKAGE = 274,
    tk_RANGE = 275,
    tk_RETURN = 276,
    tk_SELECT = 277,
    tk_STRUCT = 278,
    tk_SWITCH = 279,
    tk_TYPE = 280,
    tk_VAR = 281,
    tk_CLASS = 282,
    tk_EXTENDS = 283,
    tk_IMPLEMENTS = 284,
    tk_THIS = 285,
    tk_NEW = 286,
    tk_SUPER = 287,
    tk_PUBLIC = 288,
    tk_PRIVATE = 289,
    tk_PTRSELECT = 290,
    tk_OVERRIDE = 291,
    tk_VOID = 292,
    tk_INCLUDE = 293,
    tk_GOINCLUDE = 294,
    tk_T_STRING = 295,
    tk_T_BOOL = 296,
    tk_T_INT8 = 297,
    tk_T_UINT8 = 298,
    tk_T_BYTE = 299,
    tk_T_INT16 = 300,
    tk_T_UINT16 = 301,
    tk_T_INT32 = 302,
    tk_T_UINT32 = 303,
    tk_T_RUNE = 304,
    tk_T_INT64 = 305,
    tk_T_UINT64 = 306,
    tk_T_INT = 307,
    tk_T_UINT = 308,
    tk_T_UINTPTR = 309,
    tk_T_FLOAT32 = 310,
    tk_T_FLOAT64 = 311,
    tk_T_COMPLEX64 = 312,
    tk_T_COMPLEX128 = 313,
    tk_ADD = 314,
    tk_SUB = 315,
    tk_MUL = 316,
    tk_DIV = 317,
    tk_MOD = 318,
    tk_AND = 319,
    tk_OR = 320,
    tk_XOR = 321,
    tk_ASSIGN = 322,
    tk_LPAREN = 323,
    tk_RPAREN = 324,
    tk_LSBRACKET = 325,
    tk_RSBRACKET = 326,
    tk_LCBRACKET = 327,
    tk_RCBRACKET = 328,
    tk_COMMA = 329,
    tk_DOT = 330,
    tk_SEMI = 331,
    tk_UNDERSCORE = 332,
    tk_COLON = 333,
    tk_LSHIFT = 334,
    tk_RSHIFT = 335,
    tk_EQXOR = 336,
    tk_EQOR = 337,
    tk_EQAND = 338,
    tk_EQANDXOR = 339,
    tk_EQRSHIFT = 340,
    tk_EQLSHIFT = 341,
    tk_LOGICAND = 342,
    tk_LOGICOR = 343,
    tk_EQADD = 344,
    tk_EQSUB = 345,
    tk_EQMUL = 346,
    tk_EQDIV = 347,
    tk_EQMOD = 348,
    tk_ANDXOR = 349,
    tk_NEG = 350,
    tk_LSS = 351,
    tk_GRT = 352,
    tk_NOTEQ = 353,
    tk_EQ = 354,
    tk_EQLSS = 355,
    tk_EQGRT = 356,
    tk_SHORTDECL = 357,
    tk_ARROW = 358,
    tk_INC = 359,
    tk_DEC = 360,
    tk_ELLIPSIS = 361,
    tk_STRINGLIT = 362,
    tk_NUM = 363,
    tk_IDENT = 364,
    tk_TRUE = 365,
    tk_FALSE = 366
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 14 "grammar\\mw.y"

    char *semantic_value_ident; // for storing semantic values from stringlit, num or ident
    char *semantic_value_string;
    char *semantic_value_num;

#line 175 "src/bisonparser/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_BISONPARSER_PARSER_H_INCLUDED  */
