/* A Bison parser, made by GNU Bison 3.7.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30704

/* Bison version string.  */
#define YYBISON_VERSION "3.7.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"

/* NetHack 3.6  dgn_comp.y	$NHDT-Date: 1432512785 2015/05/25 00:13:05 $  $NHDT-Branch: master $:$NHDT-Revision: 1.8 $ */
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/*	Copyright (c) 1990 by M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Dungeon Compiler code
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive in its non-standard locale.
 */

#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "config.h"
#include "date.h"
#include "dgn_file.h"

void FDECL(yyerror, (const char *));
void FDECL(yywarning, (const char *));
int NDECL(yylex);
int NDECL(yyparse);
int FDECL(getchain, (char *));
int NDECL(check_dungeon);
int NDECL(check_branch);
int NDECL(check_level);
void NDECL(init_dungeon);
void NDECL(init_branch);
void NDECL(init_level);
void NDECL(output_dgn);

#define Free(ptr)		free((genericptr_t)ptr)

#ifdef AMIGA
# undef	printf
#ifndef	LATTICE
# define    memset(addr,val,len)    setmem(addr,len,val)
#endif
#endif

#define ERR		(-1)

static struct couple couple;
static struct tmpdungeon tmpdungeon[MAXDUNGEON];
static struct tmplevel tmplevel[LEV_LIMIT];
static struct tmpbranch tmpbranch[BRANCH_LIMIT];

static int in_dungeon = 0, n_dgns = -1, n_levs = -1, n_brs = -1;

extern int fatal_error;
extern const char *fname;
extern FILE *yyin, *yyout;	/* from dgn_lex.c */


#line 136 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "dgn_comp.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INTEGER = 3,                    /* INTEGER  */
  YYSYMBOL_A_DUNGEON = 4,                  /* A_DUNGEON  */
  YYSYMBOL_BRANCH = 5,                     /* BRANCH  */
  YYSYMBOL_CHBRANCH = 6,                   /* CHBRANCH  */
  YYSYMBOL_LEVEL = 7,                      /* LEVEL  */
  YYSYMBOL_RNDLEVEL = 8,                   /* RNDLEVEL  */
  YYSYMBOL_CHLEVEL = 9,                    /* CHLEVEL  */
  YYSYMBOL_RNDCHLEVEL = 10,                /* RNDCHLEVEL  */
  YYSYMBOL_UP_OR_DOWN = 11,                /* UP_OR_DOWN  */
  YYSYMBOL_PROTOFILE = 12,                 /* PROTOFILE  */
  YYSYMBOL_DESCRIPTION = 13,               /* DESCRIPTION  */
  YYSYMBOL_DESCRIPTOR = 14,                /* DESCRIPTOR  */
  YYSYMBOL_LEVELDESC = 15,                 /* LEVELDESC  */
  YYSYMBOL_ALIGNMENT = 16,                 /* ALIGNMENT  */
  YYSYMBOL_LEVALIGN = 17,                  /* LEVALIGN  */
  YYSYMBOL_ENTRY = 18,                     /* ENTRY  */
  YYSYMBOL_STAIR = 19,                     /* STAIR  */
  YYSYMBOL_NO_UP = 20,                     /* NO_UP  */
  YYSYMBOL_NO_DOWN = 21,                   /* NO_DOWN  */
  YYSYMBOL_PORTAL = 22,                    /* PORTAL  */
  YYSYMBOL_STRING = 23,                    /* STRING  */
  YYSYMBOL_24_ = 24,                       /* ':'  */
  YYSYMBOL_25_ = 25,                       /* '@'  */
  YYSYMBOL_26_ = 26,                       /* '+'  */
  YYSYMBOL_27_ = 27,                       /* '('  */
  YYSYMBOL_28_ = 28,                       /* ','  */
  YYSYMBOL_29_ = 29,                       /* ')'  */
  YYSYMBOL_YYACCEPT = 30,                  /* $accept  */
  YYSYMBOL_file = 31,                      /* file  */
  YYSYMBOL_dungeons = 32,                  /* dungeons  */
  YYSYMBOL_dungeon = 33,                   /* dungeon  */
  YYSYMBOL_dungeonline = 34,               /* dungeonline  */
  YYSYMBOL_optional_int = 35,              /* optional_int  */
  YYSYMBOL_dungeondesc = 36,               /* dungeondesc  */
  YYSYMBOL_entry = 37,                     /* entry  */
  YYSYMBOL_descriptions = 38,              /* descriptions  */
  YYSYMBOL_desc = 39,                      /* desc  */
  YYSYMBOL_prototype = 40,                 /* prototype  */
  YYSYMBOL_levels = 41,                    /* levels  */
  YYSYMBOL_level1 = 42,                    /* level1  */
  YYSYMBOL_level2 = 43,                    /* level2  */
  YYSYMBOL_levdesc = 44,                   /* levdesc  */
  YYSYMBOL_chlevel1 = 45,                  /* chlevel1  */
  YYSYMBOL_chlevel2 = 46,                  /* chlevel2  */
  YYSYMBOL_branches = 47,                  /* branches  */
  YYSYMBOL_branch = 48,                    /* branch  */
  YYSYMBOL_chbranch = 49,                  /* chbranch  */
  YYSYMBOL_branch_type = 50,               /* branch_type  */
  YYSYMBOL_direction = 51,                 /* direction  */
  YYSYMBOL_bones_tag = 52,                 /* bones_tag  */
  YYSYMBOL_acouple = 53,                   /* acouple  */
  YYSYMBOL_rcouple = 54                    /* rcouple  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  45
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   83

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  49
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  109

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   278


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      27,    29,     2,    26,    28,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    24,     2,
       2,     2,     2,     2,    25,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    81,    81,    82,    88,    89,    92,    93,    94,    95,
      98,   111,   114,   120,   121,   122,   125,   131,   134,   141,
     150,   157,   158,   159,   160,   161,   164,   174,   187,   198,
     212,   219,   228,   241,   257,   271,   288,   289,   292,   306,
     323,   326,   330,   334,   338,   345,   348,   354,   384,   421
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INTEGER", "A_DUNGEON",
  "BRANCH", "CHBRANCH", "LEVEL", "RNDLEVEL", "CHLEVEL", "RNDCHLEVEL",
  "UP_OR_DOWN", "PROTOFILE", "DESCRIPTION", "DESCRIPTOR", "LEVELDESC",
  "ALIGNMENT", "LEVALIGN", "ENTRY", "STAIR", "NO_UP", "NO_DOWN", "PORTAL",
  "STRING", "':'", "'@'", "'+'", "'('", "','", "')'", "$accept", "file",
  "dungeons", "dungeon", "dungeonline", "optional_int", "dungeondesc",
  "entry", "descriptions", "desc", "prototype", "levels", "level1",
  "level2", "levdesc", "chlevel1", "chlevel2", "branches", "branch",
  "chbranch", "branch_type", "direction", "bones_tag", "acouple",
  "rcouple", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    58,    64,    43,    40,    44,    41
};
#endif

#define YYPACT_NINF (-73)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -3,   -16,   -13,    -8,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    39,    -3,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,    17,    18,    19,    20,    21,    22,    23,    24,
      34,    35,    36,    37,    49,   -73,   -73,    30,    29,    32,
      30,    30,    30,    30,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,    31,    33,    38,    40,    41,    44,    45,    53,    54,
      56,     0,    31,    33,    33,    43,    46,    42,   -73,   -73,
      47,   -73,   -73,   -73,   -73,    50,     0,    59,    60,    31,
      31,    68,    70,   -73,   -73,    50,   -73,    71,    73,    74,
      51,    52,   -73,   -73,   -73,    75,   -73,   -73,   -73
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     4,     6,     7,    13,
      14,    17,    15,     9,    21,    22,    23,    24,    25,     8,
      36,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     1,     5,     0,     0,     0,
       0,     0,     0,     0,    20,    18,    30,    19,    31,    16,
      47,     0,     0,     0,     0,     0,     0,     0,     0,    11,
       0,    40,     0,     0,     0,     0,     0,     0,    12,    10,
       0,    41,    42,    43,    44,    45,    40,    26,     0,     0,
       0,     0,     0,    46,    38,    45,    28,    27,    32,     0,
       0,     0,    39,    29,    34,    33,    49,    48,    35
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -73,   -73,   -73,    64,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
      -4,   -12,   -27,   -46,   -72
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    14,    15,    16,    17,    79,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      85,    94,    61,    71,    69
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      86,     1,     2,     3,     4,     5,     6,     7,    32,     8,
       9,    33,    10,    11,    12,    13,    34,    98,    99,    81,
      82,    83,    84,    64,    65,    66,    67,    87,    88,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    62,    63,    77,    78,    68,    80,
      70,    93,    96,    97,    72,    73,    74,    75,    76,    89,
      91,   100,    90,   101,   103,    92,   104,   105,   108,    46,
     106,   107,    95,   102
};

static const yytype_int8 yycheck[] =
{
      72,     4,     5,     6,     7,     8,     9,    10,    24,    12,
      13,    24,    15,    16,    17,    18,    24,    89,    90,    19,
      20,    21,    22,    50,    51,    52,    53,    73,    74,    24,
      24,    24,    24,    24,    24,    24,    24,    24,    24,     0,
      23,    23,    23,    23,    23,    23,    23,    23,    14,    14,
      14,    14,     3,    23,    25,    23,     3,     3,    27,     3,
      27,    11,     3,     3,    26,    25,    25,    23,    23,    26,
      28,     3,    26,     3,     3,    28,     3,     3,     3,    15,
      29,    29,    86,    95
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     4,     5,     6,     7,     8,     9,    10,    12,    13,
      15,    16,    17,    18,    31,    32,    33,    34,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    24,    24,    24,    24,    24,    24,    24,    24,
      24,    24,    24,    24,    24,     0,    33,    23,    23,    23,
      23,    23,    23,    23,    23,    14,    14,    14,    14,     3,
      23,    52,    25,    23,    52,    52,    52,    52,    27,    54,
      27,    53,    26,    25,    25,    23,    23,     3,     3,    35,
       3,    19,    20,    21,    22,    50,    54,    53,    53,    26,
      26,    28,    28,    11,    51,    50,     3,     3,    54,    54,
       3,     3,    51,     3,     3,     3,    29,    29,     3
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    30,    31,    31,    32,    32,    33,    33,    33,    33,
      34,    35,    35,    36,    36,    36,    37,    38,    39,    39,
      40,    41,    41,    41,    41,    41,    42,    42,    43,    43,
      44,    44,    45,    45,    46,    46,    47,    47,    48,    49,
      50,    50,    50,    50,    50,    51,    51,    52,    53,    54
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     1,     1,     1,
       6,     0,     1,     1,     1,     1,     3,     1,     3,     3,
       3,     1,     1,     1,     1,     1,     6,     7,     7,     8,
       3,     3,     7,     8,     8,     9,     1,     1,     7,     8,
       0,     1,     1,     1,     1,     0,     1,     1,     5,     5
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* file: dungeons  */
#line 83 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			output_dgn();
		  }
#line 1234 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 10: /* dungeonline: A_DUNGEON ':' STRING bones_tag rcouple optional_int  */
#line 99 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_dungeon();
			Strcpy(tmpdungeon[n_dgns].name, (yyvsp[-3].str));
			tmpdungeon[n_dgns].boneschar = (char)(yyvsp[-2].i);
			tmpdungeon[n_dgns].lev.base = couple.base;
			tmpdungeon[n_dgns].lev.rand = couple.rand;
			tmpdungeon[n_dgns].chance = (yyvsp[0].i);
			Free((yyvsp[-3].str));
		  }
#line 1248 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 11: /* optional_int: %empty  */
#line 111 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = 0;
		  }
#line 1256 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 12: /* optional_int: INTEGER  */
#line 115 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = (yyvsp[0].i);
		  }
#line 1264 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 16: /* entry: ENTRY ':' INTEGER  */
#line 126 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			tmpdungeon[n_dgns].entry_lev = (yyvsp[0].i);
		  }
#line 1272 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 18: /* desc: DESCRIPTION ':' DESCRIPTOR  */
#line 135 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			if((yyvsp[0].i) <= TOWN || (yyvsp[0].i) >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmpdungeon[n_dgns].tflags |= (yyvsp[0].i) ;
		  }
#line 1283 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 19: /* desc: ALIGNMENT ':' DESCRIPTOR  */
#line 142 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			if((yyvsp[0].i) && (yyvsp[0].i) < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmpdungeon[n_dgns].tflags |= (yyvsp[0].i) ;
		  }
#line 1294 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 20: /* prototype: PROTOFILE ':' STRING  */
#line 151 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			Strcpy(tmpdungeon[n_dgns].protoname, (yyvsp[0].str));
			Free((yyvsp[0].str));
		  }
#line 1303 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 26: /* level1: LEVEL ':' STRING bones_tag '@' acouple  */
#line 165 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-3].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-2].i);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-3].str));
		  }
#line 1317 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 27: /* level1: RNDLEVEL ':' STRING bones_tag '@' acouple INTEGER  */
#line 175 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-4].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-3].i);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = (yyvsp[0].i);
			tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-4].str));
		  }
#line 1332 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 28: /* level2: LEVEL ':' STRING bones_tag '@' acouple INTEGER  */
#line 188 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-4].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-3].i);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = (yyvsp[0].i);
			tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-4].str));
		  }
#line 1347 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 29: /* level2: RNDLEVEL ':' STRING bones_tag '@' acouple INTEGER INTEGER  */
#line 199 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-5].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-4].i);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = (yyvsp[-1].i);
			tmplevel[n_levs].rndlevs = (yyvsp[0].i);
			tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-5].str));
		  }
#line 1363 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 30: /* levdesc: LEVELDESC ':' DESCRIPTOR  */
#line 213 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			if((yyvsp[0].i) >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmplevel[n_levs].tflags |= (yyvsp[0].i) ;
		  }
#line 1374 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 31: /* levdesc: LEVALIGN ':' DESCRIPTOR  */
#line 220 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			if((yyvsp[0].i) && (yyvsp[0].i) < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmplevel[n_levs].tflags |= (yyvsp[0].i) ;
		  }
#line 1385 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 32: /* chlevel1: CHLEVEL ':' STRING bones_tag STRING '+' rcouple  */
#line 229 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-4].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-3].i);
			tmplevel[n_levs].chain = getchain((yyvsp[-2].str));
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-4].str));
			Free((yyvsp[-2].str));
		  }
#line 1402 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 33: /* chlevel1: RNDCHLEVEL ':' STRING bones_tag STRING '+' rcouple INTEGER  */
#line 242 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-5].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-4].i);
			tmplevel[n_levs].chain = getchain((yyvsp[-3].str));
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = (yyvsp[0].i);
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-5].str));
			Free((yyvsp[-3].str));
		  }
#line 1420 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 34: /* chlevel2: CHLEVEL ':' STRING bones_tag STRING '+' rcouple INTEGER  */
#line 258 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-5].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-4].i);
			tmplevel[n_levs].chain = getchain((yyvsp[-3].str));
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = (yyvsp[0].i);
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-5].str));
			Free((yyvsp[-3].str));
		  }
#line 1438 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 35: /* chlevel2: RNDCHLEVEL ':' STRING bones_tag STRING '+' rcouple INTEGER INTEGER  */
#line 272 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_level();
			Strcpy(tmplevel[n_levs].name, (yyvsp[-6].str));
			tmplevel[n_levs].boneschar = (char)(yyvsp[-5].i);
			tmplevel[n_levs].chain = getchain((yyvsp[-4].str));
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = (yyvsp[-1].i);
			tmplevel[n_levs].rndlevs = (yyvsp[0].i);
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
			Free((yyvsp[-6].str));
			Free((yyvsp[-4].str));
		  }
#line 1457 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 38: /* branch: BRANCH ':' STRING '@' acouple branch_type direction  */
#line 293 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_branch();
			Strcpy(tmpbranch[n_brs].name, (yyvsp[-4].str));
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = (yyvsp[-1].i);
			tmpbranch[n_brs].up = (yyvsp[0].i);
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
			Free((yyvsp[-4].str));
		  }
#line 1473 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 39: /* chbranch: CHBRANCH ':' STRING STRING '+' rcouple branch_type direction  */
#line 307 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			init_branch();
			Strcpy(tmpbranch[n_brs].name, (yyvsp[-5].str));
			tmpbranch[n_brs].chain = getchain((yyvsp[-4].str));
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = (yyvsp[-1].i);
			tmpbranch[n_brs].up = (yyvsp[0].i);
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
			Free((yyvsp[-5].str));
			Free((yyvsp[-4].str));
		  }
#line 1491 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 40: /* branch_type: %empty  */
#line 323 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = TBR_STAIR;	/* two way stair */
		  }
#line 1499 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 41: /* branch_type: STAIR  */
#line 327 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = TBR_STAIR;	/* two way stair */
		  }
#line 1507 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 42: /* branch_type: NO_UP  */
#line 331 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = TBR_NO_UP;	/* no up staircase */
		  }
#line 1515 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 43: /* branch_type: NO_DOWN  */
#line 335 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = TBR_NO_DOWN;	/* no down staircase */
		  }
#line 1523 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 44: /* branch_type: PORTAL  */
#line 339 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = TBR_PORTAL;	/* portal connection */
		  }
#line 1531 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 45: /* direction: %empty  */
#line 345 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = 0;	/* defaults to down */
		  }
#line 1539 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 46: /* direction: UP_OR_DOWN  */
#line 349 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			(yyval.i) = (yyvsp[0].i);
		  }
#line 1547 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 47: /* bones_tag: STRING  */
#line 355 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			char *p = (yyvsp[0].str);
			if (strlen(p) != 1) {
			    if (strcmp(p, "none") != 0)
		   yyerror("Bones marker must be a single char, or \"none\"!");
			    *p = '\0';
			}
			(yyval.i) = *p;
			Free(p);
		  }
#line 1562 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 48: /* acouple: '(' INTEGER ',' INTEGER ')'  */
#line 385 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			if ((yyvsp[-3].i) < -MAXLEVEL || (yyvsp[-3].i) > MAXLEVEL) {
			    yyerror("Abs base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else if ((yyvsp[-1].i) < -1 ||
				(((yyvsp[-3].i) < 0) ? (MAXLEVEL + (yyvsp[-3].i) + (yyvsp[-1].i) + 1) > MAXLEVEL :
					((yyvsp[-3].i) + (yyvsp[-1].i)) > MAXLEVEL)) {
			    yyerror("Abs range out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = (yyvsp[-3].i);
			    couple.rand = (yyvsp[-1].i);
			}
		  }
#line 1581 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;

  case 49: /* rcouple: '(' INTEGER ',' INTEGER ')'  */
#line 422 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"
                  {
			if ((yyvsp[-3].i) < -MAXLEVEL || (yyvsp[-3].i) > MAXLEVEL) {
			    yyerror("Rel base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = (yyvsp[-3].i);
			    couple.rand = (yyvsp[-1].i);
			}
		  }
#line 1595 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"
    break;


#line 1599 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 432 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/dgn_comp.y"


void
init_dungeon()
{
	if(++n_dgns > MAXDUNGEON) {
	    (void) fprintf(stderr, "FATAL - Too many dungeons (limit: %d).\n",
		    MAXDUNGEON);
	    (void) fprintf(stderr, "To increase the limit edit MAXDUNGEON in global.h\n");
	    exit(EXIT_FAILURE);
	}

	in_dungeon = 1;
	tmpdungeon[n_dgns].lev.base = 0;
	tmpdungeon[n_dgns].lev.rand = 0;
	tmpdungeon[n_dgns].chance = 100;
	Strcpy(tmpdungeon[n_dgns].name, "");
	Strcpy(tmpdungeon[n_dgns].protoname, "");
	tmpdungeon[n_dgns].tflags = 0;
	tmpdungeon[n_dgns].levels = 0;
	tmpdungeon[n_dgns].branches = 0;
	tmpdungeon[n_dgns].entry_lev = 0;
}

void
init_level()
{
	if(++n_levs > LEV_LIMIT) {

		yyerror("FATAL - Too many special levels defined.");
		exit(EXIT_FAILURE);
	}
	tmplevel[n_levs].lev.base = 0;
	tmplevel[n_levs].lev.rand = 0;
	tmplevel[n_levs].chance = 100;
	tmplevel[n_levs].rndlevs = 0;
	tmplevel[n_levs].tflags = 0;
	Strcpy(tmplevel[n_levs].name, "");
	tmplevel[n_levs].chain = -1;
}

void
init_branch()
{
	if(++n_brs > BRANCH_LIMIT) {

		yyerror("FATAL - Too many special levels defined.");
		exit(EXIT_FAILURE);
	}
	tmpbranch[n_brs].lev.base = 0;
	tmpbranch[n_brs].lev.rand = 0;
	Strcpy(tmpbranch[n_brs].name, "");
	tmpbranch[n_brs].chain = -1;
}

int
getchain(s)
	char	*s;
{
	int i;

	if(strlen(s)) {

	    for(i = n_levs - tmpdungeon[n_dgns].levels + 1; i <= n_levs; i++)
		if(!strcmp(tmplevel[i].name, s)) return i;

	    yyerror("Can't locate the specified chain level.");
	    return(-2);
	}
	return(-1);
}

/*
 *	Consistancy checking routines:
 *
 *	- A dungeon must have a unique name.
 *	- A dungeon must have a originating "branch" command
 *	  (except, of course, for the first dungeon).
 *	- A dungeon must have a proper depth (at least (1, 0)).
 */

int
check_dungeon()
{
	int i;

	for(i = 0; i < n_dgns; i++)
	    if(!strcmp(tmpdungeon[i].name, tmpdungeon[n_dgns].name)) {
		yyerror("Duplicate dungeon name.");
		return(0);
	    }

	if(n_dgns)
	  for(i = 0; i < n_brs - tmpdungeon[n_dgns].branches; i++) {
	    if(!strcmp(tmpbranch[i].name, tmpdungeon[n_dgns].name)) break;

	    if(i >= n_brs - tmpdungeon[n_dgns].branches) {
		yyerror("Dungeon cannot be reached.");
		return(0);
	    }
	  }

	if(tmpdungeon[n_dgns].lev.base <= 0 ||
	   tmpdungeon[n_dgns].lev.rand < 0) {
		yyerror("Invalid dungeon depth specified.");
		return(0);
	}
	return(1);	/* OK */
}

/*
 *	- A level must have a unique level name.
 *	- If chained, the level used as reference for the chain
 *	  must be in this dungeon, must be previously defined, and
 *	  the level chained from must be "non-probabilistic" (ie.
 *	  have a 100% chance of existing).
 */

int
check_level()
{
	int i;

	if(!in_dungeon) {
		yyerror("Level defined outside of dungeon.");
		return(0);
	}

	for(i = 0; i < n_levs; i++)
	    if(!strcmp(tmplevel[i].name, tmplevel[n_levs].name)) {
		yyerror("Duplicate level name.");
		return(0);
	    }

	if(tmplevel[i].chain == -2) {
		yyerror("Invaild level chain reference.");
		return(0);
	} else if(tmplevel[i].chain != -1) {	/* there is a chain */
	    /* KMH -- tmplevel[tmpbranch[i].chain].chance was in error */
	    if(tmplevel[tmplevel[i].chain].chance != 100) {
		yyerror("Level cannot chain from a probabilistic level.");
		return(0);
	    } else if(tmplevel[i].chain == n_levs) {
		yyerror("A level cannot chain to itself!");
		return(0);
	    }
	}
	return(1);	/* OK */
}

/*
 *	- A branch may not branch backwards - to avoid branch loops.
 *	- A branch name must be unique.
 *	  (ie. You can only have one entry point to each dungeon).
 *	- If chained, the level used as reference for the chain
 *	  must be in this dungeon, must be previously defined, and
 *	  the level chained from must be "non-probabilistic" (ie.
 *	  have a 100% chance of existing).
 */

int
check_branch()
{
	int i;

	if(!in_dungeon) {
		yyerror("Branch defined outside of dungeon.");
		return(0);
	}

	for(i = 0; i < n_dgns; i++)
	    if(!strcmp(tmpdungeon[i].name, tmpbranch[n_brs].name)) {

		yyerror("Reverse branching not allowed.");
		return(0);
	    }

	if(tmpbranch[i].chain == -2) {

		yyerror("Invaild branch chain reference.");
		return(0);
	} else if(tmpbranch[i].chain != -1) {	/* it is chained */

	    if(tmplevel[tmpbranch[i].chain].chance != 100) {
		yyerror("Branch cannot chain from a probabilistic level.");
		return(0);
	    }
	}
	return(1);	/* OK */
}

/*
 *	Output the dungon definition into a file.
 *
 *	The file will have the following format:
 *
 *	[ nethack version ID ]
 *	[ number of dungeons ]
 *	[ first dungeon struct ]
 *	[ levels for the first dungeon ]
 *	  ...
 *	[ branches for the first dungeon ]
 *	  ...
 *	[ second dungeon struct ]
 *	  ...
 */

void
output_dgn()
{
	int	nd, cl = 0, nl = 0,
		    cb = 0, nb = 0;
	static struct version_info version_data = {
			VERSION_NUMBER, VERSION_FEATURES,
			VERSION_SANITY1, VERSION_SANITY2, VERSION_SANITY3
	};

	if(++n_dgns <= 0) {
	    yyerror("FATAL - no dungeons were defined.");
	    exit(EXIT_FAILURE);
	}

	if (fwrite((char *)&version_data, sizeof version_data, 1, yyout) != 1) {
	    yyerror("FATAL - output failure.");
	    exit(EXIT_FAILURE);
	}

	(void) fwrite((char *)&n_dgns, sizeof(int), 1, yyout);
	for (nd = 0; nd < n_dgns; nd++) {
	    (void) fwrite((char *)&tmpdungeon[nd], sizeof(struct tmpdungeon),
							1, yyout);

	    nl += tmpdungeon[nd].levels;
	    for(; cl < nl; cl++)
		(void) fwrite((char *)&tmplevel[cl], sizeof(struct tmplevel),
							1, yyout);

	    nb += tmpdungeon[nd].branches;
	    for(; cb < nb; cb++)
		(void) fwrite((char *)&tmpbranch[cb], sizeof(struct tmpbranch),
							1, yyout);
	}
	/* apparently necessary for Think C 5.x, otherwise harmless */
	(void) fflush(yyout);
}

/*dgn_comp.y*/
