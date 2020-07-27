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
    tk_INTERFACE = 271,
    tk_MAP = 272,
    tk_PACKAGE = 273,
    tk_RANGE = 274,
    tk_RETURN = 275,
    tk_SELECT = 276,
    tk_STRUCT = 277,
    tk_SWITCH = 278,
    tk_TYPE = 279,
    tk_VAR = 280,
    tk_CLASS = 281,
    tk_EXTENDS = 282,
    tk_IMPLEMENTS = 283,
    tk_THIS = 284,
    tk_NEW = 285,
    tk_SUPER = 286,
    tk_PUBLIC = 287,
    tk_PRIVATE = 288,
    tk_PTRSELECT = 289,
    tk_OVERRIDE = 290,
    tk_VOID = 291,
    tk_IMPORT = 292,
    tk_GOIMPORT = 293,
    tk_T_STRING = 294,
    tk_T_BOOL = 295,
    tk_T_INT8 = 296,
    tk_T_UINT8 = 297,
    tk_T_BYTE = 298,
    tk_T_INT16 = 299,
    tk_T_UINT16 = 300,
    tk_T_INT32 = 301,
    tk_T_UINT32 = 302,
    tk_T_RUNE = 303,
    tk_T_INT64 = 304,
    tk_T_UINT64 = 305,
    tk_T_INT = 306,
    tk_T_UINT = 307,
    tk_T_UINTPTR = 308,
    tk_T_FLOAT32 = 309,
    tk_T_FLOAT64 = 310,
    tk_T_COMPLEX64 = 311,
    tk_T_COMPLEX128 = 312,
    tk_ADD = 313,
    tk_SUB = 314,
    tk_MUL = 315,
    tk_DIV = 316,
    tk_MOD = 317,
    tk_AND = 318,
    tk_OR = 319,
    tk_XOR = 320,
    tk_ASSIGN = 321,
    tk_LPAREN = 322,
    tk_RPAREN = 323,
    tk_LSBRACKET = 324,
    tk_RSBRACKET = 325,
    tk_LCBRACKET = 326,
    tk_RCBRACKET = 327,
    tk_COMMA = 328,
    tk_DOT = 329,
    tk_SEMI = 330,
    tk_UNDERSCORE = 331,
    tk_COLON = 332,
    tk_LSHIFT = 333,
    tk_RSHIFT = 334,
    tk_EQXOR = 335,
    tk_EQOR = 336,
    tk_EQAND = 337,
    tk_EQANDXOR = 338,
    tk_EQRSHIFT = 339,
    tk_EQLSHIFT = 340,
    tk_LOGICAND = 341,
    tk_LOGICOR = 342,
    tk_EQADD = 343,
    tk_EQSUB = 344,
    tk_EQMUL = 345,
    tk_EQDIV = 346,
    tk_EQMOD = 347,
    tk_ANDXOR = 348,
    tk_NEG = 349,
    tk_LSS = 350,
    tk_GRT = 351,
    tk_NOTEQ = 352,
    tk_EQ = 353,
    tk_EQLSS = 354,
    tk_EQGRT = 355,
    tk_SHORTDECL = 356,
    tk_ARROW = 357,
    tk_INC = 358,
    tk_DEC = 359,
    tk_ELLIPSIS = 360,
    tk_STRINGLIT = 361,
    tk_NUM = 362,
    tk_IDENT = 363,
    tk_TRUE = 364,
    tk_FALSE = 365,
    tk_IOTA = 366
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 11 "grammar\\mw.y"

    char *semantic_value; // for storing semantic values from stringlit, num or ident

#line 173 "src/bisonparser/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_BISONPARSER_PARSER_H_INCLUDED  */
