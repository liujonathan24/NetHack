/* A Bison parser, made by GNU Bison 3.7.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SCRATCH_GPFS_ZHUANGL_JL0796_PUFFERLIB_VENDOR_NLE_SRC_BUILD_UTIL_DGN_COMP_H_INCLUDED
# define YY_YY_SCRATCH_GPFS_ZHUANGL_JL0796_PUFFERLIB_VENDOR_NLE_SRC_BUILD_UTIL_DGN_COMP_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INTEGER = 258,                 /* INTEGER  */
    A_DUNGEON = 259,               /* A_DUNGEON  */
    BRANCH = 260,                  /* BRANCH  */
    CHBRANCH = 261,                /* CHBRANCH  */
    LEVEL = 262,                   /* LEVEL  */
    RNDLEVEL = 263,                /* RNDLEVEL  */
    CHLEVEL = 264,                 /* CHLEVEL  */
    RNDCHLEVEL = 265,              /* RNDCHLEVEL  */
    UP_OR_DOWN = 266,              /* UP_OR_DOWN  */
    PROTOFILE = 267,               /* PROTOFILE  */
    DESCRIPTION = 268,             /* DESCRIPTION  */
    DESCRIPTOR = 269,              /* DESCRIPTOR  */
    LEVELDESC = 270,               /* LEVELDESC  */
    ALIGNMENT = 271,               /* ALIGNMENT  */
    LEVALIGN = 272,                /* LEVALIGN  */
    ENTRY = 273,                   /* ENTRY  */
    STAIR = 274,                   /* STAIR  */
    NO_UP = 275,                   /* NO_UP  */
    NO_DOWN = 276,                 /* NO_DOWN  */
    PORTAL = 277,                  /* PORTAL  */
    STRING = 278                   /* STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define INTEGER 258
#define A_DUNGEON 259
#define BRANCH 260
#define CHBRANCH 261
#define LEVEL 262
#define RNDLEVEL 263
#define CHLEVEL 264
#define RNDCHLEVEL 265
#define UP_OR_DOWN 266
#define PROTOFILE 267
#define DESCRIPTION 268
#define DESCRIPTOR 269
#define LEVELDESC 270
#define ALIGNMENT 271
#define LEVALIGN 272
#define ENTRY 273
#define STAIR 274
#define NO_UP 275
#define NO_DOWN 276
#define PORTAL 277
#define STRING 278

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 67 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"

	int	i;
	char*	str;

#line 118 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_comp.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SCRATCH_GPFS_ZHUANGL_JL0796_PUFFERLIB_VENDOR_NLE_SRC_BUILD_UTIL_DGN_COMP_H_INCLUDED  */
