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
#line 1 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"

/* NetHack 3.6  lev_comp.y	$NHDT-Date: 1543371691 2018/11/28 02:21:31 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.22 $ */
/*      Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive.
 */
#ifdef _AIX
 #pragma alloca         /* keep leading space! */
#endif

#define SPEC_LEV    /* for USE_OLDARGS (sp_lev.h) */
#include "hack.h"
#include "sp_lev.h"

#define ERR             (-1)
/* many types of things are put in chars for transference to NetHack.
 * since some systems will use signed chars, limit everybody to the
 * same number for portability.
 */
#define MAX_OF_TYPE     128

#define MAX_NESTED_IFS   20
#define MAX_SWITCH_CASES 20

#define New(type) \
        (type *) memset((genericptr_t) alloc(sizeof (type)), 0, sizeof (type))
#define NewTab(type, size)      (type **) alloc(sizeof (type *) * size)
#define Free(ptr)               free((genericptr_t) ptr)

extern void VDECL(lc_error, (const char *, ...));
extern void VDECL(lc_warning, (const char *, ...));
extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);

extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *,CHAR_P));
extern int FDECL(get_object_id, (char *,CHAR_P));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *, sp_lev *));
extern void FDECL(add_opcode, (sp_lev *, int, genericptr_t));
extern genericptr_t FDECL(get_last_opcode_data1, (sp_lev *, int));
extern genericptr_t FDECL(get_last_opcode_data2, (sp_lev *, int, int));
extern boolean FDECL(check_subrooms, (sp_lev *));
extern boolean FDECL(write_level_file, (char *,sp_lev *));
extern struct opvar *FDECL(set_opvar_int, (struct opvar *, long));
extern void VDECL(add_opvars, (sp_lev *, const char *, ...));
extern void FDECL(start_level_def, (sp_lev * *, char *));

extern struct lc_funcdefs *FDECL(funcdef_new, (long,char *));
extern void FDECL(funcdef_free_all, (struct lc_funcdefs *));
extern struct lc_funcdefs *FDECL(funcdef_defined, (struct lc_funcdefs *,
                                                   char *, int));
extern char *FDECL(funcdef_paramtypes, (struct lc_funcdefs *));
extern char *FDECL(decode_parm_str, (char *));

extern struct lc_vardefs *FDECL(vardef_new, (long,char *));
extern void FDECL(vardef_free_all, (struct lc_vardefs *));
extern struct lc_vardefs *FDECL(vardef_defined, (struct lc_vardefs *,
                                                 char *, int));

extern void NDECL(break_stmt_start);
extern void FDECL(break_stmt_end, (sp_lev *));
extern void FDECL(break_stmt_new, (sp_lev *, long));

extern void FDECL(splev_add_from, (sp_lev *, sp_lev *));

extern void FDECL(check_vardef_type, (struct lc_vardefs *, char *, long));
extern void FDECL(vardef_used, (struct lc_vardefs *, char *));
extern struct lc_vardefs *FDECL(add_vardef_type, (struct lc_vardefs *,
                                                  char *, long));

extern int FDECL(reverse_jmp_opcode, (int));

struct coord {
    long x;
    long y;
};

struct forloopdef {
    char *varname;
    long jmp_point;
};
static struct forloopdef forloop_list[MAX_NESTED_IFS];
static short n_forloops = 0;


sp_lev *splev = NULL;

static struct opvar *if_list[MAX_NESTED_IFS];

static short n_if_list = 0;

unsigned int max_x_map, max_y_map;
int obj_containment = 0;

int in_container_obj = 0;

/* integer value is possibly an inconstant value (eg. dice notation
   or a variable) */
int is_inconstant_number = 0;

int in_switch_statement = 0;
static struct opvar *switch_check_jump = NULL;
static struct opvar *switch_default_case = NULL;
static struct opvar *switch_case_list[MAX_SWITCH_CASES];
static long switch_case_value[MAX_SWITCH_CASES];
int n_switch_case_list = 0;

int allow_break_statements = 0;
struct lc_breakdef *break_list = NULL;

extern struct lc_vardefs *vardefs; /* variable definitions */


struct lc_vardefs *function_tmp_var_defs = NULL;
extern struct lc_funcdefs *function_definitions;
struct lc_funcdefs *curr_function = NULL;
struct lc_funcdefs_parm * curr_function_param = NULL;
int in_function_definition = 0;
sp_lev *function_splev_backup = NULL;

extern int fatal_error;
extern int got_errors;
extern int line_number;
extern const char *fname;

extern char curr_token[512];


#line 220 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"

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

#include "lev_comp.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_CHAR = 3,                       /* CHAR  */
  YYSYMBOL_INTEGER = 4,                    /* INTEGER  */
  YYSYMBOL_BOOLEAN = 5,                    /* BOOLEAN  */
  YYSYMBOL_PERCENT = 6,                    /* PERCENT  */
  YYSYMBOL_SPERCENT = 7,                   /* SPERCENT  */
  YYSYMBOL_MINUS_INTEGER = 8,              /* MINUS_INTEGER  */
  YYSYMBOL_PLUS_INTEGER = 9,               /* PLUS_INTEGER  */
  YYSYMBOL_MAZE_GRID_ID = 10,              /* MAZE_GRID_ID  */
  YYSYMBOL_SOLID_FILL_ID = 11,             /* SOLID_FILL_ID  */
  YYSYMBOL_MINES_ID = 12,                  /* MINES_ID  */
  YYSYMBOL_ROGUELEV_ID = 13,               /* ROGUELEV_ID  */
  YYSYMBOL_MESSAGE_ID = 14,                /* MESSAGE_ID  */
  YYSYMBOL_MAZE_ID = 15,                   /* MAZE_ID  */
  YYSYMBOL_LEVEL_ID = 16,                  /* LEVEL_ID  */
  YYSYMBOL_LEV_INIT_ID = 17,               /* LEV_INIT_ID  */
  YYSYMBOL_GEOMETRY_ID = 18,               /* GEOMETRY_ID  */
  YYSYMBOL_NOMAP_ID = 19,                  /* NOMAP_ID  */
  YYSYMBOL_OBJECT_ID = 20,                 /* OBJECT_ID  */
  YYSYMBOL_COBJECT_ID = 21,                /* COBJECT_ID  */
  YYSYMBOL_MONSTER_ID = 22,                /* MONSTER_ID  */
  YYSYMBOL_TRAP_ID = 23,                   /* TRAP_ID  */
  YYSYMBOL_DOOR_ID = 24,                   /* DOOR_ID  */
  YYSYMBOL_DRAWBRIDGE_ID = 25,             /* DRAWBRIDGE_ID  */
  YYSYMBOL_object_ID = 26,                 /* object_ID  */
  YYSYMBOL_monster_ID = 27,                /* monster_ID  */
  YYSYMBOL_terrain_ID = 28,                /* terrain_ID  */
  YYSYMBOL_MAZEWALK_ID = 29,               /* MAZEWALK_ID  */
  YYSYMBOL_WALLIFY_ID = 30,                /* WALLIFY_ID  */
  YYSYMBOL_REGION_ID = 31,                 /* REGION_ID  */
  YYSYMBOL_FILLING = 32,                   /* FILLING  */
  YYSYMBOL_IRREGULAR = 33,                 /* IRREGULAR  */
  YYSYMBOL_JOINED = 34,                    /* JOINED  */
  YYSYMBOL_ALTAR_ID = 35,                  /* ALTAR_ID  */
  YYSYMBOL_LADDER_ID = 36,                 /* LADDER_ID  */
  YYSYMBOL_STAIR_ID = 37,                  /* STAIR_ID  */
  YYSYMBOL_NON_DIGGABLE_ID = 38,           /* NON_DIGGABLE_ID  */
  YYSYMBOL_NON_PASSWALL_ID = 39,           /* NON_PASSWALL_ID  */
  YYSYMBOL_ROOM_ID = 40,                   /* ROOM_ID  */
  YYSYMBOL_PORTAL_ID = 41,                 /* PORTAL_ID  */
  YYSYMBOL_TELEPRT_ID = 42,                /* TELEPRT_ID  */
  YYSYMBOL_BRANCH_ID = 43,                 /* BRANCH_ID  */
  YYSYMBOL_LEV = 44,                       /* LEV  */
  YYSYMBOL_MINERALIZE_ID = 45,             /* MINERALIZE_ID  */
  YYSYMBOL_CORRIDOR_ID = 46,               /* CORRIDOR_ID  */
  YYSYMBOL_GOLD_ID = 47,                   /* GOLD_ID  */
  YYSYMBOL_ENGRAVING_ID = 48,              /* ENGRAVING_ID  */
  YYSYMBOL_FOUNTAIN_ID = 49,               /* FOUNTAIN_ID  */
  YYSYMBOL_POOL_ID = 50,                   /* POOL_ID  */
  YYSYMBOL_SINK_ID = 51,                   /* SINK_ID  */
  YYSYMBOL_NONE = 52,                      /* NONE  */
  YYSYMBOL_RAND_CORRIDOR_ID = 53,          /* RAND_CORRIDOR_ID  */
  YYSYMBOL_DOOR_STATE = 54,                /* DOOR_STATE  */
  YYSYMBOL_LIGHT_STATE = 55,               /* LIGHT_STATE  */
  YYSYMBOL_CURSE_TYPE = 56,                /* CURSE_TYPE  */
  YYSYMBOL_ENGRAVING_TYPE = 57,            /* ENGRAVING_TYPE  */
  YYSYMBOL_DIRECTION = 58,                 /* DIRECTION  */
  YYSYMBOL_RANDOM_TYPE = 59,               /* RANDOM_TYPE  */
  YYSYMBOL_RANDOM_TYPE_BRACKET = 60,       /* RANDOM_TYPE_BRACKET  */
  YYSYMBOL_A_REGISTER = 61,                /* A_REGISTER  */
  YYSYMBOL_ALIGNMENT = 62,                 /* ALIGNMENT  */
  YYSYMBOL_LEFT_OR_RIGHT = 63,             /* LEFT_OR_RIGHT  */
  YYSYMBOL_CENTER = 64,                    /* CENTER  */
  YYSYMBOL_TOP_OR_BOT = 65,                /* TOP_OR_BOT  */
  YYSYMBOL_ALTAR_TYPE = 66,                /* ALTAR_TYPE  */
  YYSYMBOL_UP_OR_DOWN = 67,                /* UP_OR_DOWN  */
  YYSYMBOL_SUBROOM_ID = 68,                /* SUBROOM_ID  */
  YYSYMBOL_NAME_ID = 69,                   /* NAME_ID  */
  YYSYMBOL_FLAGS_ID = 70,                  /* FLAGS_ID  */
  YYSYMBOL_FLAG_TYPE = 71,                 /* FLAG_TYPE  */
  YYSYMBOL_MON_ATTITUDE = 72,              /* MON_ATTITUDE  */
  YYSYMBOL_MON_ALERTNESS = 73,             /* MON_ALERTNESS  */
  YYSYMBOL_MON_APPEARANCE = 74,            /* MON_APPEARANCE  */
  YYSYMBOL_ROOMDOOR_ID = 75,               /* ROOMDOOR_ID  */
  YYSYMBOL_IF_ID = 76,                     /* IF_ID  */
  YYSYMBOL_ELSE_ID = 77,                   /* ELSE_ID  */
  YYSYMBOL_TERRAIN_ID = 78,                /* TERRAIN_ID  */
  YYSYMBOL_HORIZ_OR_VERT = 79,             /* HORIZ_OR_VERT  */
  YYSYMBOL_REPLACE_TERRAIN_ID = 80,        /* REPLACE_TERRAIN_ID  */
  YYSYMBOL_EXIT_ID = 81,                   /* EXIT_ID  */
  YYSYMBOL_SHUFFLE_ID = 82,                /* SHUFFLE_ID  */
  YYSYMBOL_QUANTITY_ID = 83,               /* QUANTITY_ID  */
  YYSYMBOL_BURIED_ID = 84,                 /* BURIED_ID  */
  YYSYMBOL_LOOP_ID = 85,                   /* LOOP_ID  */
  YYSYMBOL_FOR_ID = 86,                    /* FOR_ID  */
  YYSYMBOL_TO_ID = 87,                     /* TO_ID  */
  YYSYMBOL_SWITCH_ID = 88,                 /* SWITCH_ID  */
  YYSYMBOL_CASE_ID = 89,                   /* CASE_ID  */
  YYSYMBOL_BREAK_ID = 90,                  /* BREAK_ID  */
  YYSYMBOL_DEFAULT_ID = 91,                /* DEFAULT_ID  */
  YYSYMBOL_ERODED_ID = 92,                 /* ERODED_ID  */
  YYSYMBOL_TRAPPED_STATE = 93,             /* TRAPPED_STATE  */
  YYSYMBOL_RECHARGED_ID = 94,              /* RECHARGED_ID  */
  YYSYMBOL_INVIS_ID = 95,                  /* INVIS_ID  */
  YYSYMBOL_GREASED_ID = 96,                /* GREASED_ID  */
  YYSYMBOL_FEMALE_ID = 97,                 /* FEMALE_ID  */
  YYSYMBOL_CANCELLED_ID = 98,              /* CANCELLED_ID  */
  YYSYMBOL_REVIVED_ID = 99,                /* REVIVED_ID  */
  YYSYMBOL_AVENGE_ID = 100,                /* AVENGE_ID  */
  YYSYMBOL_FLEEING_ID = 101,               /* FLEEING_ID  */
  YYSYMBOL_BLINDED_ID = 102,               /* BLINDED_ID  */
  YYSYMBOL_PARALYZED_ID = 103,             /* PARALYZED_ID  */
  YYSYMBOL_STUNNED_ID = 104,               /* STUNNED_ID  */
  YYSYMBOL_CONFUSED_ID = 105,              /* CONFUSED_ID  */
  YYSYMBOL_SEENTRAPS_ID = 106,             /* SEENTRAPS_ID  */
  YYSYMBOL_ALL_ID = 107,                   /* ALL_ID  */
  YYSYMBOL_MONTYPE_ID = 108,               /* MONTYPE_ID  */
  YYSYMBOL_GRAVE_ID = 109,                 /* GRAVE_ID  */
  YYSYMBOL_ERODEPROOF_ID = 110,            /* ERODEPROOF_ID  */
  YYSYMBOL_FUNCTION_ID = 111,              /* FUNCTION_ID  */
  YYSYMBOL_MSG_OUTPUT_TYPE = 112,          /* MSG_OUTPUT_TYPE  */
  YYSYMBOL_COMPARE_TYPE = 113,             /* COMPARE_TYPE  */
  YYSYMBOL_UNKNOWN_TYPE = 114,             /* UNKNOWN_TYPE  */
  YYSYMBOL_rect_ID = 115,                  /* rect_ID  */
  YYSYMBOL_fillrect_ID = 116,              /* fillrect_ID  */
  YYSYMBOL_line_ID = 117,                  /* line_ID  */
  YYSYMBOL_randline_ID = 118,              /* randline_ID  */
  YYSYMBOL_grow_ID = 119,                  /* grow_ID  */
  YYSYMBOL_selection_ID = 120,             /* selection_ID  */
  YYSYMBOL_flood_ID = 121,                 /* flood_ID  */
  YYSYMBOL_rndcoord_ID = 122,              /* rndcoord_ID  */
  YYSYMBOL_circle_ID = 123,                /* circle_ID  */
  YYSYMBOL_ellipse_ID = 124,               /* ellipse_ID  */
  YYSYMBOL_filter_ID = 125,                /* filter_ID  */
  YYSYMBOL_complement_ID = 126,            /* complement_ID  */
  YYSYMBOL_gradient_ID = 127,              /* gradient_ID  */
  YYSYMBOL_GRADIENT_TYPE = 128,            /* GRADIENT_TYPE  */
  YYSYMBOL_LIMITED = 129,                  /* LIMITED  */
  YYSYMBOL_HUMIDITY_TYPE = 130,            /* HUMIDITY_TYPE  */
  YYSYMBOL_131_ = 131,                     /* ','  */
  YYSYMBOL_132_ = 132,                     /* ':'  */
  YYSYMBOL_133_ = 133,                     /* '('  */
  YYSYMBOL_134_ = 134,                     /* ')'  */
  YYSYMBOL_135_ = 135,                     /* '['  */
  YYSYMBOL_136_ = 136,                     /* ']'  */
  YYSYMBOL_137_ = 137,                     /* '{'  */
  YYSYMBOL_138_ = 138,                     /* '}'  */
  YYSYMBOL_STRING = 139,                   /* STRING  */
  YYSYMBOL_MAP_ID = 140,                   /* MAP_ID  */
  YYSYMBOL_NQSTRING = 141,                 /* NQSTRING  */
  YYSYMBOL_VARSTRING = 142,                /* VARSTRING  */
  YYSYMBOL_CFUNC = 143,                    /* CFUNC  */
  YYSYMBOL_CFUNC_INT = 144,                /* CFUNC_INT  */
  YYSYMBOL_CFUNC_STR = 145,                /* CFUNC_STR  */
  YYSYMBOL_CFUNC_COORD = 146,              /* CFUNC_COORD  */
  YYSYMBOL_CFUNC_REGION = 147,             /* CFUNC_REGION  */
  YYSYMBOL_VARSTRING_INT = 148,            /* VARSTRING_INT  */
  YYSYMBOL_VARSTRING_INT_ARRAY = 149,      /* VARSTRING_INT_ARRAY  */
  YYSYMBOL_VARSTRING_STRING = 150,         /* VARSTRING_STRING  */
  YYSYMBOL_VARSTRING_STRING_ARRAY = 151,   /* VARSTRING_STRING_ARRAY  */
  YYSYMBOL_VARSTRING_VAR = 152,            /* VARSTRING_VAR  */
  YYSYMBOL_VARSTRING_VAR_ARRAY = 153,      /* VARSTRING_VAR_ARRAY  */
  YYSYMBOL_VARSTRING_COORD = 154,          /* VARSTRING_COORD  */
  YYSYMBOL_VARSTRING_COORD_ARRAY = 155,    /* VARSTRING_COORD_ARRAY  */
  YYSYMBOL_VARSTRING_REGION = 156,         /* VARSTRING_REGION  */
  YYSYMBOL_VARSTRING_REGION_ARRAY = 157,   /* VARSTRING_REGION_ARRAY  */
  YYSYMBOL_VARSTRING_MAPCHAR = 158,        /* VARSTRING_MAPCHAR  */
  YYSYMBOL_VARSTRING_MAPCHAR_ARRAY = 159,  /* VARSTRING_MAPCHAR_ARRAY  */
  YYSYMBOL_VARSTRING_MONST = 160,          /* VARSTRING_MONST  */
  YYSYMBOL_VARSTRING_MONST_ARRAY = 161,    /* VARSTRING_MONST_ARRAY  */
  YYSYMBOL_VARSTRING_OBJ = 162,            /* VARSTRING_OBJ  */
  YYSYMBOL_VARSTRING_OBJ_ARRAY = 163,      /* VARSTRING_OBJ_ARRAY  */
  YYSYMBOL_VARSTRING_SEL = 164,            /* VARSTRING_SEL  */
  YYSYMBOL_VARSTRING_SEL_ARRAY = 165,      /* VARSTRING_SEL_ARRAY  */
  YYSYMBOL_METHOD_INT = 166,               /* METHOD_INT  */
  YYSYMBOL_METHOD_INT_ARRAY = 167,         /* METHOD_INT_ARRAY  */
  YYSYMBOL_METHOD_STRING = 168,            /* METHOD_STRING  */
  YYSYMBOL_METHOD_STRING_ARRAY = 169,      /* METHOD_STRING_ARRAY  */
  YYSYMBOL_METHOD_VAR = 170,               /* METHOD_VAR  */
  YYSYMBOL_METHOD_VAR_ARRAY = 171,         /* METHOD_VAR_ARRAY  */
  YYSYMBOL_METHOD_COORD = 172,             /* METHOD_COORD  */
  YYSYMBOL_METHOD_COORD_ARRAY = 173,       /* METHOD_COORD_ARRAY  */
  YYSYMBOL_METHOD_REGION = 174,            /* METHOD_REGION  */
  YYSYMBOL_METHOD_REGION_ARRAY = 175,      /* METHOD_REGION_ARRAY  */
  YYSYMBOL_METHOD_MAPCHAR = 176,           /* METHOD_MAPCHAR  */
  YYSYMBOL_METHOD_MAPCHAR_ARRAY = 177,     /* METHOD_MAPCHAR_ARRAY  */
  YYSYMBOL_METHOD_MONST = 178,             /* METHOD_MONST  */
  YYSYMBOL_METHOD_MONST_ARRAY = 179,       /* METHOD_MONST_ARRAY  */
  YYSYMBOL_METHOD_OBJ = 180,               /* METHOD_OBJ  */
  YYSYMBOL_METHOD_OBJ_ARRAY = 181,         /* METHOD_OBJ_ARRAY  */
  YYSYMBOL_METHOD_SEL = 182,               /* METHOD_SEL  */
  YYSYMBOL_METHOD_SEL_ARRAY = 183,         /* METHOD_SEL_ARRAY  */
  YYSYMBOL_DICE = 184,                     /* DICE  */
  YYSYMBOL_185_ = 185,                     /* '+'  */
  YYSYMBOL_186_ = 186,                     /* '-'  */
  YYSYMBOL_187_ = 187,                     /* '*'  */
  YYSYMBOL_188_ = 188,                     /* '/'  */
  YYSYMBOL_189_ = 189,                     /* '%'  */
  YYSYMBOL_190_ = 190,                     /* '='  */
  YYSYMBOL_191_ = 191,                     /* '.'  */
  YYSYMBOL_192_ = 192,                     /* '|'  */
  YYSYMBOL_193_ = 193,                     /* '&'  */
  YYSYMBOL_YYACCEPT = 194,                 /* $accept  */
  YYSYMBOL_file = 195,                     /* file  */
  YYSYMBOL_levels = 196,                   /* levels  */
  YYSYMBOL_level = 197,                    /* level  */
  YYSYMBOL_level_def = 198,                /* level_def  */
  YYSYMBOL_mazefiller = 199,               /* mazefiller  */
  YYSYMBOL_lev_init = 200,                 /* lev_init  */
  YYSYMBOL_opt_limited = 201,              /* opt_limited  */
  YYSYMBOL_opt_coord_or_var = 202,         /* opt_coord_or_var  */
  YYSYMBOL_opt_fillchar = 203,             /* opt_fillchar  */
  YYSYMBOL_walled = 204,                   /* walled  */
  YYSYMBOL_flags = 205,                    /* flags  */
  YYSYMBOL_flag_list = 206,                /* flag_list  */
  YYSYMBOL_levstatements = 207,            /* levstatements  */
  YYSYMBOL_stmt_block = 208,               /* stmt_block  */
  YYSYMBOL_levstatement = 209,             /* levstatement  */
  YYSYMBOL_any_var_array = 210,            /* any_var_array  */
  YYSYMBOL_any_var = 211,                  /* any_var  */
  YYSYMBOL_any_var_or_arr = 212,           /* any_var_or_arr  */
  YYSYMBOL_any_var_or_unk = 213,           /* any_var_or_unk  */
  YYSYMBOL_shuffle_detail = 214,           /* shuffle_detail  */
  YYSYMBOL_variable_define = 215,          /* variable_define  */
  YYSYMBOL_encodeobj_list = 216,           /* encodeobj_list  */
  YYSYMBOL_encodemonster_list = 217,       /* encodemonster_list  */
  YYSYMBOL_mapchar_list = 218,             /* mapchar_list  */
  YYSYMBOL_encoderegion_list = 219,        /* encoderegion_list  */
  YYSYMBOL_encodecoord_list = 220,         /* encodecoord_list  */
  YYSYMBOL_integer_list = 221,             /* integer_list  */
  YYSYMBOL_string_list = 222,              /* string_list  */
  YYSYMBOL_function_define = 223,          /* function_define  */
  YYSYMBOL_224_1 = 224,                    /* $@1  */
  YYSYMBOL_225_2 = 225,                    /* $@2  */
  YYSYMBOL_function_call = 226,            /* function_call  */
  YYSYMBOL_exitstatement = 227,            /* exitstatement  */
  YYSYMBOL_opt_percent = 228,              /* opt_percent  */
  YYSYMBOL_comparestmt = 229,              /* comparestmt  */
  YYSYMBOL_switchstatement = 230,          /* switchstatement  */
  YYSYMBOL_231_3 = 231,                    /* $@3  */
  YYSYMBOL_232_4 = 232,                    /* $@4  */
  YYSYMBOL_switchcases = 233,              /* switchcases  */
  YYSYMBOL_switchcase = 234,               /* switchcase  */
  YYSYMBOL_235_5 = 235,                    /* $@5  */
  YYSYMBOL_236_6 = 236,                    /* $@6  */
  YYSYMBOL_breakstatement = 237,           /* breakstatement  */
  YYSYMBOL_for_to_span = 238,              /* for_to_span  */
  YYSYMBOL_forstmt_start = 239,            /* forstmt_start  */
  YYSYMBOL_forstatement = 240,             /* forstatement  */
  YYSYMBOL_241_7 = 241,                    /* $@7  */
  YYSYMBOL_loopstatement = 242,            /* loopstatement  */
  YYSYMBOL_243_8 = 243,                    /* $@8  */
  YYSYMBOL_chancestatement = 244,          /* chancestatement  */
  YYSYMBOL_245_9 = 245,                    /* $@9  */
  YYSYMBOL_ifstatement = 246,              /* ifstatement  */
  YYSYMBOL_247_10 = 247,                   /* $@10  */
  YYSYMBOL_if_ending = 248,                /* if_ending  */
  YYSYMBOL_249_11 = 249,                   /* $@11  */
  YYSYMBOL_message = 250,                  /* message  */
  YYSYMBOL_random_corridors = 251,         /* random_corridors  */
  YYSYMBOL_corridor = 252,                 /* corridor  */
  YYSYMBOL_corr_spec = 253,                /* corr_spec  */
  YYSYMBOL_room_begin = 254,               /* room_begin  */
  YYSYMBOL_subroom_def = 255,              /* subroom_def  */
  YYSYMBOL_256_12 = 256,                   /* $@12  */
  YYSYMBOL_room_def = 257,                 /* room_def  */
  YYSYMBOL_258_13 = 258,                   /* $@13  */
  YYSYMBOL_roomfill = 259,                 /* roomfill  */
  YYSYMBOL_room_pos = 260,                 /* room_pos  */
  YYSYMBOL_subroom_pos = 261,              /* subroom_pos  */
  YYSYMBOL_room_align = 262,               /* room_align  */
  YYSYMBOL_room_size = 263,                /* room_size  */
  YYSYMBOL_door_detail = 264,              /* door_detail  */
  YYSYMBOL_secret = 265,                   /* secret  */
  YYSYMBOL_door_wall = 266,                /* door_wall  */
  YYSYMBOL_dir_list = 267,                 /* dir_list  */
  YYSYMBOL_door_pos = 268,                 /* door_pos  */
  YYSYMBOL_map_definition = 269,           /* map_definition  */
  YYSYMBOL_h_justif = 270,                 /* h_justif  */
  YYSYMBOL_v_justif = 271,                 /* v_justif  */
  YYSYMBOL_monster_detail = 272,           /* monster_detail  */
  YYSYMBOL_273_14 = 273,                   /* $@14  */
  YYSYMBOL_monster_desc = 274,             /* monster_desc  */
  YYSYMBOL_monster_infos = 275,            /* monster_infos  */
  YYSYMBOL_monster_info = 276,             /* monster_info  */
  YYSYMBOL_seen_trap_mask = 277,           /* seen_trap_mask  */
  YYSYMBOL_object_detail = 278,            /* object_detail  */
  YYSYMBOL_279_15 = 279,                   /* $@15  */
  YYSYMBOL_object_desc = 280,              /* object_desc  */
  YYSYMBOL_object_infos = 281,             /* object_infos  */
  YYSYMBOL_object_info = 282,              /* object_info  */
  YYSYMBOL_trap_detail = 283,              /* trap_detail  */
  YYSYMBOL_drawbridge_detail = 284,        /* drawbridge_detail  */
  YYSYMBOL_mazewalk_detail = 285,          /* mazewalk_detail  */
  YYSYMBOL_wallify_detail = 286,           /* wallify_detail  */
  YYSYMBOL_ladder_detail = 287,            /* ladder_detail  */
  YYSYMBOL_stair_detail = 288,             /* stair_detail  */
  YYSYMBOL_stair_region = 289,             /* stair_region  */
  YYSYMBOL_portal_region = 290,            /* portal_region  */
  YYSYMBOL_teleprt_region = 291,           /* teleprt_region  */
  YYSYMBOL_branch_region = 292,            /* branch_region  */
  YYSYMBOL_teleprt_detail = 293,           /* teleprt_detail  */
  YYSYMBOL_fountain_detail = 294,          /* fountain_detail  */
  YYSYMBOL_sink_detail = 295,              /* sink_detail  */
  YYSYMBOL_pool_detail = 296,              /* pool_detail  */
  YYSYMBOL_terrain_type = 297,             /* terrain_type  */
  YYSYMBOL_replace_terrain_detail = 298,   /* replace_terrain_detail  */
  YYSYMBOL_terrain_detail = 299,           /* terrain_detail  */
  YYSYMBOL_diggable_detail = 300,          /* diggable_detail  */
  YYSYMBOL_passwall_detail = 301,          /* passwall_detail  */
  YYSYMBOL_region_detail = 302,            /* region_detail  */
  YYSYMBOL_303_16 = 303,                   /* @16  */
  YYSYMBOL_region_detail_end = 304,        /* region_detail_end  */
  YYSYMBOL_altar_detail = 305,             /* altar_detail  */
  YYSYMBOL_grave_detail = 306,             /* grave_detail  */
  YYSYMBOL_gold_detail = 307,              /* gold_detail  */
  YYSYMBOL_engraving_detail = 308,         /* engraving_detail  */
  YYSYMBOL_mineralize = 309,               /* mineralize  */
  YYSYMBOL_trap_name = 310,                /* trap_name  */
  YYSYMBOL_room_type = 311,                /* room_type  */
  YYSYMBOL_optroomregionflags = 312,       /* optroomregionflags  */
  YYSYMBOL_roomregionflags = 313,          /* roomregionflags  */
  YYSYMBOL_roomregionflag = 314,           /* roomregionflag  */
  YYSYMBOL_door_state = 315,               /* door_state  */
  YYSYMBOL_light_state = 316,              /* light_state  */
  YYSYMBOL_alignment = 317,                /* alignment  */
  YYSYMBOL_alignment_prfx = 318,           /* alignment_prfx  */
  YYSYMBOL_altar_type = 319,               /* altar_type  */
  YYSYMBOL_a_register = 320,               /* a_register  */
  YYSYMBOL_string_or_var = 321,            /* string_or_var  */
  YYSYMBOL_integer_or_var = 322,           /* integer_or_var  */
  YYSYMBOL_coord_or_var = 323,             /* coord_or_var  */
  YYSYMBOL_encodecoord = 324,              /* encodecoord  */
  YYSYMBOL_humidity_flags = 325,           /* humidity_flags  */
  YYSYMBOL_region_or_var = 326,            /* region_or_var  */
  YYSYMBOL_encoderegion = 327,             /* encoderegion  */
  YYSYMBOL_mapchar_or_var = 328,           /* mapchar_or_var  */
  YYSYMBOL_mapchar = 329,                  /* mapchar  */
  YYSYMBOL_monster_or_var = 330,           /* monster_or_var  */
  YYSYMBOL_encodemonster = 331,            /* encodemonster  */
  YYSYMBOL_object_or_var = 332,            /* object_or_var  */
  YYSYMBOL_encodeobj = 333,                /* encodeobj  */
  YYSYMBOL_string_expr = 334,              /* string_expr  */
  YYSYMBOL_math_expr_var = 335,            /* math_expr_var  */
  YYSYMBOL_func_param_type = 336,          /* func_param_type  */
  YYSYMBOL_func_param_part = 337,          /* func_param_part  */
  YYSYMBOL_func_param_list = 338,          /* func_param_list  */
  YYSYMBOL_func_params_list = 339,         /* func_params_list  */
  YYSYMBOL_func_call_param_part = 340,     /* func_call_param_part  */
  YYSYMBOL_func_call_param_list = 341,     /* func_call_param_list  */
  YYSYMBOL_func_call_params_list = 342,    /* func_call_params_list  */
  YYSYMBOL_ter_selection_x = 343,          /* ter_selection_x  */
  YYSYMBOL_ter_selection = 344,            /* ter_selection  */
  YYSYMBOL_dice = 345,                     /* dice  */
  YYSYMBOL_all_integers = 346,             /* all_integers  */
  YYSYMBOL_all_ints_push = 347,            /* all_ints_push  */
  YYSYMBOL_objectid = 348,                 /* objectid  */
  YYSYMBOL_monsterid = 349,                /* monsterid  */
  YYSYMBOL_terrainid = 350,                /* terrainid  */
  YYSYMBOL_engraving_type = 351,           /* engraving_type  */
  YYSYMBOL_lev_region = 352,               /* lev_region  */
  YYSYMBOL_region = 353                    /* region  */
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
typedef yytype_int16 yy_state_t;

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
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1033

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  194
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  160
/* YYNRULES -- Number of rules.  */
#define YYNRULES  406
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  866

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   431


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   189,   193,     2,
     133,   134,   187,   185,   131,   186,   191,   188,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   132,     2,
       2,   190,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   135,     2,   136,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   137,   192,   138,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   281,   281,   282,   285,   286,   289,   312,   317,   339,
     343,   349,   362,   374,   380,   410,   413,   420,   424,   431,
     434,   441,   442,   446,   449,   456,   460,   467,   470,   476,
     482,   483,   484,   485,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   495,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   507,   508,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   528,   529,   530,   531,   532,   533,
     534,   535,   536,   539,   540,   541,   542,   543,   544,   545,
     546,   547,   550,   551,   552,   555,   556,   559,   575,   581,
     587,   593,   599,   605,   611,   617,   623,   633,   643,   653,
     663,   673,   683,   695,   700,   707,   712,   719,   724,   731,
     735,   741,   746,   753,   757,   763,   767,   774,   796,   773,
     810,   865,   872,   875,   881,   888,   892,   901,   905,   900,
     968,   969,   973,   972,   986,   985,  1000,  1010,  1011,  1014,
    1052,  1051,  1086,  1085,  1116,  1115,  1148,  1147,  1173,  1184,
    1183,  1211,  1217,  1222,  1227,  1234,  1241,  1250,  1258,  1270,
    1269,  1288,  1287,  1306,  1309,  1315,  1325,  1331,  1340,  1346,
    1351,  1357,  1362,  1368,  1379,  1385,  1386,  1389,  1390,  1393,
    1397,  1403,  1404,  1407,  1414,  1422,  1430,  1431,  1434,  1435,
    1438,  1443,  1442,  1456,  1463,  1470,  1478,  1483,  1489,  1495,
    1501,  1507,  1512,  1517,  1522,  1527,  1532,  1537,  1542,  1547,
    1552,  1557,  1565,  1574,  1578,  1591,  1600,  1599,  1617,  1627,
    1633,  1641,  1647,  1652,  1657,  1662,  1667,  1672,  1677,  1682,
    1687,  1698,  1704,  1709,  1714,  1719,  1726,  1732,  1761,  1766,
    1774,  1780,  1786,  1793,  1800,  1810,  1820,  1835,  1846,  1849,
    1855,  1861,  1867,  1873,  1878,  1885,  1892,  1898,  1904,  1911,
    1910,  1935,  1938,  1944,  1951,  1955,  1960,  1967,  1973,  1980,
    1984,  1991,  1999,  2002,  2012,  2016,  2019,  2025,  2029,  2036,
    2040,  2044,  2050,  2051,  2054,  2055,  2058,  2059,  2060,  2066,
    2067,  2068,  2074,  2075,  2078,  2087,  2092,  2099,  2110,  2116,
    2120,  2124,  2131,  2141,  2148,  2152,  2158,  2162,  2170,  2174,
    2181,  2191,  2204,  2208,  2215,  2225,  2234,  2245,  2249,  2256,
    2266,  2277,  2286,  2296,  2302,  2306,  2313,  2323,  2334,  2343,
    2353,  2360,  2361,  2367,  2371,  2375,  2379,  2387,  2396,  2400,
    2404,  2408,  2412,  2416,  2419,  2426,  2435,  2468,  2469,  2472,
    2473,  2476,  2480,  2487,  2494,  2505,  2508,  2516,  2520,  2524,
    2528,  2532,  2537,  2541,  2545,  2550,  2555,  2560,  2564,  2569,
    2574,  2578,  2582,  2587,  2591,  2598,  2604,  2608,  2614,  2621,
    2622,  2623,  2626,  2630,  2634,  2638,  2644,  2645,  2648,  2649,
    2652,  2653,  2656,  2657,  2660,  2664,  2690
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
  "\"end of file\"", "error", "\"invalid token\"", "CHAR", "INTEGER",
  "BOOLEAN", "PERCENT", "SPERCENT", "MINUS_INTEGER", "PLUS_INTEGER",
  "MAZE_GRID_ID", "SOLID_FILL_ID", "MINES_ID", "ROGUELEV_ID", "MESSAGE_ID",
  "MAZE_ID", "LEVEL_ID", "LEV_INIT_ID", "GEOMETRY_ID", "NOMAP_ID",
  "OBJECT_ID", "COBJECT_ID", "MONSTER_ID", "TRAP_ID", "DOOR_ID",
  "DRAWBRIDGE_ID", "object_ID", "monster_ID", "terrain_ID", "MAZEWALK_ID",
  "WALLIFY_ID", "REGION_ID", "FILLING", "IRREGULAR", "JOINED", "ALTAR_ID",
  "LADDER_ID", "STAIR_ID", "NON_DIGGABLE_ID", "NON_PASSWALL_ID", "ROOM_ID",
  "PORTAL_ID", "TELEPRT_ID", "BRANCH_ID", "LEV", "MINERALIZE_ID",
  "CORRIDOR_ID", "GOLD_ID", "ENGRAVING_ID", "FOUNTAIN_ID", "POOL_ID",
  "SINK_ID", "NONE", "RAND_CORRIDOR_ID", "DOOR_STATE", "LIGHT_STATE",
  "CURSE_TYPE", "ENGRAVING_TYPE", "DIRECTION", "RANDOM_TYPE",
  "RANDOM_TYPE_BRACKET", "A_REGISTER", "ALIGNMENT", "LEFT_OR_RIGHT",
  "CENTER", "TOP_OR_BOT", "ALTAR_TYPE", "UP_OR_DOWN", "SUBROOM_ID",
  "NAME_ID", "FLAGS_ID", "FLAG_TYPE", "MON_ATTITUDE", "MON_ALERTNESS",
  "MON_APPEARANCE", "ROOMDOOR_ID", "IF_ID", "ELSE_ID", "TERRAIN_ID",
  "HORIZ_OR_VERT", "REPLACE_TERRAIN_ID", "EXIT_ID", "SHUFFLE_ID",
  "QUANTITY_ID", "BURIED_ID", "LOOP_ID", "FOR_ID", "TO_ID", "SWITCH_ID",
  "CASE_ID", "BREAK_ID", "DEFAULT_ID", "ERODED_ID", "TRAPPED_STATE",
  "RECHARGED_ID", "INVIS_ID", "GREASED_ID", "FEMALE_ID", "CANCELLED_ID",
  "REVIVED_ID", "AVENGE_ID", "FLEEING_ID", "BLINDED_ID", "PARALYZED_ID",
  "STUNNED_ID", "CONFUSED_ID", "SEENTRAPS_ID", "ALL_ID", "MONTYPE_ID",
  "GRAVE_ID", "ERODEPROOF_ID", "FUNCTION_ID", "MSG_OUTPUT_TYPE",
  "COMPARE_TYPE", "UNKNOWN_TYPE", "rect_ID", "fillrect_ID", "line_ID",
  "randline_ID", "grow_ID", "selection_ID", "flood_ID", "rndcoord_ID",
  "circle_ID", "ellipse_ID", "filter_ID", "complement_ID", "gradient_ID",
  "GRADIENT_TYPE", "LIMITED", "HUMIDITY_TYPE", "','", "':'", "'('", "')'",
  "'['", "']'", "'{'", "'}'", "STRING", "MAP_ID", "NQSTRING", "VARSTRING",
  "CFUNC", "CFUNC_INT", "CFUNC_STR", "CFUNC_COORD", "CFUNC_REGION",
  "VARSTRING_INT", "VARSTRING_INT_ARRAY", "VARSTRING_STRING",
  "VARSTRING_STRING_ARRAY", "VARSTRING_VAR", "VARSTRING_VAR_ARRAY",
  "VARSTRING_COORD", "VARSTRING_COORD_ARRAY", "VARSTRING_REGION",
  "VARSTRING_REGION_ARRAY", "VARSTRING_MAPCHAR", "VARSTRING_MAPCHAR_ARRAY",
  "VARSTRING_MONST", "VARSTRING_MONST_ARRAY", "VARSTRING_OBJ",
  "VARSTRING_OBJ_ARRAY", "VARSTRING_SEL", "VARSTRING_SEL_ARRAY",
  "METHOD_INT", "METHOD_INT_ARRAY", "METHOD_STRING", "METHOD_STRING_ARRAY",
  "METHOD_VAR", "METHOD_VAR_ARRAY", "METHOD_COORD", "METHOD_COORD_ARRAY",
  "METHOD_REGION", "METHOD_REGION_ARRAY", "METHOD_MAPCHAR",
  "METHOD_MAPCHAR_ARRAY", "METHOD_MONST", "METHOD_MONST_ARRAY",
  "METHOD_OBJ", "METHOD_OBJ_ARRAY", "METHOD_SEL", "METHOD_SEL_ARRAY",
  "DICE", "'+'", "'-'", "'*'", "'/'", "'%'", "'='", "'.'", "'|'", "'&'",
  "$accept", "file", "levels", "level", "level_def", "mazefiller",
  "lev_init", "opt_limited", "opt_coord_or_var", "opt_fillchar", "walled",
  "flags", "flag_list", "levstatements", "stmt_block", "levstatement",
  "any_var_array", "any_var", "any_var_or_arr", "any_var_or_unk",
  "shuffle_detail", "variable_define", "encodeobj_list",
  "encodemonster_list", "mapchar_list", "encoderegion_list",
  "encodecoord_list", "integer_list", "string_list", "function_define",
  "$@1", "$@2", "function_call", "exitstatement", "opt_percent",
  "comparestmt", "switchstatement", "$@3", "$@4", "switchcases",
  "switchcase", "$@5", "$@6", "breakstatement", "for_to_span",
  "forstmt_start", "forstatement", "$@7", "loopstatement", "$@8",
  "chancestatement", "$@9", "ifstatement", "$@10", "if_ending", "$@11",
  "message", "random_corridors", "corridor", "corr_spec", "room_begin",
  "subroom_def", "$@12", "room_def", "$@13", "roomfill", "room_pos",
  "subroom_pos", "room_align", "room_size", "door_detail", "secret",
  "door_wall", "dir_list", "door_pos", "map_definition", "h_justif",
  "v_justif", "monster_detail", "$@14", "monster_desc", "monster_infos",
  "monster_info", "seen_trap_mask", "object_detail", "$@15", "object_desc",
  "object_infos", "object_info", "trap_detail", "drawbridge_detail",
  "mazewalk_detail", "wallify_detail", "ladder_detail", "stair_detail",
  "stair_region", "portal_region", "teleprt_region", "branch_region",
  "teleprt_detail", "fountain_detail", "sink_detail", "pool_detail",
  "terrain_type", "replace_terrain_detail", "terrain_detail",
  "diggable_detail", "passwall_detail", "region_detail", "@16",
  "region_detail_end", "altar_detail", "grave_detail", "gold_detail",
  "engraving_detail", "mineralize", "trap_name", "room_type",
  "optroomregionflags", "roomregionflags", "roomregionflag", "door_state",
  "light_state", "alignment", "alignment_prfx", "altar_type", "a_register",
  "string_or_var", "integer_or_var", "coord_or_var", "encodecoord",
  "humidity_flags", "region_or_var", "encoderegion", "mapchar_or_var",
  "mapchar", "monster_or_var", "encodemonster", "object_or_var",
  "encodeobj", "string_expr", "math_expr_var", "func_param_type",
  "func_param_part", "func_param_list", "func_params_list",
  "func_call_param_part", "func_call_param_list", "func_call_params_list",
  "ter_selection_x", "ter_selection", "dice", "all_integers",
  "all_ints_push", "objectid", "monsterid", "terrainid", "engraving_type",
  "lev_region", "region", YY_NULLPTR
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
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,    44,    58,    40,    41,    91,    93,   123,   125,   386,
     387,   388,   389,   390,   391,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   412,   413,   414,   415,   416,
     417,   418,   419,   420,   421,   422,   423,   424,   425,   426,
     427,   428,   429,   430,   431,    43,    45,    42,    47,    37,
      61,    46,   124,    38
};
#endif

#define YYPACT_NINF (-654)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-202)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     137,     7,    27,    83,  -654,   137,    24,   -15,    37,  -654,
    -654,    91,   733,   -10,  -654,   113,  -654,    99,   131,   133,
    -654,   161,   164,   166,   174,   183,   199,   214,   231,   233,
     239,   241,   243,   245,   247,   250,   268,   269,   275,   276,
     281,   295,   305,   317,   322,   323,   325,   326,   327,    28,
     340,   343,  -654,   345,   205,   757,  -654,  -654,   346,    49,
      66,   265,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,   733,  -654,  -654,   190,  -654,  -654,  -654,
    -654,  -654,   353,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,    59,   308,  -654,   -31,   406,   351,    58,    58,
     144,   -11,    57,   -18,   -18,   672,   -57,   -18,   -18,   248,
     -57,   -57,    -6,    -1,    -1,    -1,    66,   303,    66,   -18,
     672,   672,   672,   319,    -6,    51,  -654,   672,   -57,   751,
      66,  -654,  -654,   279,   339,   -18,   355,  -654,    29,  -654,
     344,  -654,   198,  -654,    56,  -654,    18,  -654,   359,  -654,
    -654,  -654,   113,  -654,  -654,   360,  -654,   309,   368,   371,
     372,  -654,  -654,   374,  -654,  -654,   375,   503,  -654,   378,
     379,   383,  -654,  -654,  -654,   506,  -654,  -654,   402,  -654,
    -654,  -654,  -654,  -654,  -654,   537,  -654,  -654,   413,   404,
     418,  -654,  -654,  -654,   420,  -654,  -654,   427,   439,   446,
     -57,   -57,   -18,   -18,   417,   -18,   436,   445,   449,   672,
     450,   508,  -654,  -654,   391,  -654,   582,  -654,   453,   458,
    -654,   459,   460,   466,   599,   473,   474,  -654,  -654,  -654,
    -654,  -654,   475,   601,   608,   482,   483,   489,   490,   387,
     618,   526,   208,   529,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,   530,  -654,  -654,   534,   359,   538,   539,  -654,
     523,    66,    66,   540,  -654,   548,   342,    66,    66,  -654,
      66,    66,    66,    66,    66,   309,   387,  -654,   542,   549,
    -654,  -654,  -654,  -654,  -654,  -654,   554,    60,    32,  -654,
    -654,   309,   387,   555,   556,   558,   733,   733,  -654,  -654,
      66,   -31,   671,    46,   698,   571,   567,   672,   573,    66,
     215,   700,   566,   581,    66,   587,   359,   589,    66,   359,
     -18,   -18,   672,   663,   664,  -654,  -654,   592,   593,   521,
    -654,   -18,   -18,   307,  -654,   598,   596,   672,   597,    66,
      67,   186,   660,   730,   604,   669,    -1,     8,  -654,   606,
     607,    -1,    -1,    -1,    66,   609,    89,   -18,   141,    12,
      57,   665,  -654,    52,    52,  -654,   156,   605,   -38,   697,
    -654,  -654,   331,   347,   168,   168,  -654,  -654,  -654,    56,
    -654,   672,   612,   -52,   -50,     4,   140,  -654,  -654,   309,
     387,    55,   127,   158,  -654,   611,   358,  -654,  -654,  -654,
     743,  -654,   628,   374,  -654,   626,   761,   430,  -654,  -654,
     383,  -654,  -654,   627,   464,   266,  -654,   638,   492,  -654,
    -654,  -654,  -654,   636,   654,   -18,   -18,   600,   673,   666,
     675,   676,  -654,   679,   438,  -654,   667,   681,  -654,   685,
     686,  -654,  -654,   799,   522,  -654,  -654,   689,  -654,   687,
    -654,   693,  -654,  -654,   694,   824,  -654,   699,  -654,   825,
     701,    67,   827,   702,   703,  -654,   704,   779,  -654,  -654,
    -654,  -654,  -654,   707,  -654,   836,   710,   712,   786,   861,
    -654,   734,   359,  -654,   678,    66,  -654,  -654,   309,   735,
    -654,   739,   732,  -654,  -654,  -654,  -654,   867,   740,  -654,
      -8,  -654,    66,  -654,   -31,  -654,    21,  -654,    25,  -654,
      54,  -654,  -654,  -654,   741,   873,  -654,  -654,   744,  -654,
     737,   745,  -654,  -654,  -654,  -654,  -654,  -654,  -654,   748,
     769,  -654,   771,  -654,   788,  -654,  -654,   790,  -654,  -654,
    -654,  -654,  -654,   784,  -654,   792,    57,   919,  -654,   794,
     868,   672,  -654,    66,    66,   672,   796,    66,   672,   672,
     795,   798,  -654,    -6,   926,    90,   927,   -49,   865,   802,
      13,  -654,   803,   797,   870,  -654,    66,   804,   -31,   807,
      15,   254,   359,    52,  -654,  -654,   387,   805,   224,   697,
    -654,   -29,  -654,  -654,   387,   309,   151,  -654,   159,  -654,
     171,  -654,    67,   808,  -654,  -654,  -654,   -31,    66,    66,
      66,   144,  -654,   594,  -654,   809,    66,  -654,   810,   258,
     337,   811,    67,   528,   812,   813,    66,   937,   817,   814,
    -654,  -654,  -654,   818,   939,  -654,   947,  -654,   289,   821,
    -654,  -654,   822,    85,   309,   950,  -654,   951,   817,  -654,
     826,  -654,  -654,   828,   160,  -654,  -654,  -654,  -654,   359,
      21,  -654,    25,  -654,    54,  -654,   829,   953,   309,  -654,
    -654,  -654,  -654,   149,  -654,  -654,  -654,   -31,  -654,  -654,
    -654,  -654,  -654,   830,   832,   833,  -654,  -654,   834,  -654,
    -654,  -654,   309,   957,  -654,   387,  -654,   924,  -654,    66,
    -654,   835,  -654,  -654,  -654,   409,   837,   419,  -654,  -654,
     963,   839,   838,   840,    15,    66,  -654,  -654,   841,   842,
     843,  -654,    85,   954,   329,   845,   844,   160,  -654,  -654,
    -654,  -654,  -654,   847,   914,   309,    66,    66,    66,   -44,
    -654,   846,   304,  -654,    66,   975,  -654,  -654,  -654,  -654,
     850,   359,   852,   980,  -654,   215,   817,  -654,  -654,  -654,
     981,   359,  -654,  -654,   854,  -654,  -654,  -654,   982,  -654,
    -654,  -654,  -654,  -654,   800,  -654,  -654,   956,  -654,   217,
     855,   419,  -654,  -654,   986,   857,   859,  -654,   860,  -654,
    -654,   733,   864,   -44,   862,   869,   863,  -654,  -654,   866,
    -654,  -654,   359,  -654,   733,  -654,    67,  -654,  -654,  -654,
     871,  -654,  -654,  -654,   872,   -18,    68,   874,  -654,  -654,
     809,   -18,   875,  -654,  -654,  -654
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     0,     0,     3,     4,    23,     0,     0,     1,
       5,     0,    27,     0,     7,     0,   134,     0,     0,     0,
     193,     0,     0,     0,     0,     0,     0,     0,   250,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   280,
       0,     0,     0,     0,     0,     0,   162,     0,     0,     0,
       0,     0,   131,     0,     0,     0,   137,   146,     0,     0,
       0,     0,    94,    83,    74,    84,    75,    85,    76,    86,
      77,    87,    78,    88,    79,    89,    80,    90,    81,    91,
      82,    31,     6,    27,    92,    93,     0,    37,    36,    52,
      53,    50,     0,    45,    51,   150,    46,    47,    49,    48,
      30,    62,    35,    65,    64,    39,    55,    57,    58,    72,
      40,    56,    73,    54,    69,    70,    61,    71,    34,    43,
      66,    60,    68,    67,    38,    59,    63,    32,    33,    44,
      41,    42,     0,    26,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   156,     0,     0,     0,
       0,    95,    96,     0,     0,     0,     0,   343,     0,   346,
       0,   388,     0,   344,   365,    28,     0,   154,     0,    10,
       9,     8,     0,   305,   306,     0,   341,   161,     0,     0,
       0,    13,   314,     0,   196,   197,     0,     0,   311,     0,
       0,   173,   309,   338,   340,     0,   337,   335,     0,   225,
     229,   334,   226,   331,   333,     0,   330,   328,     0,   200,
       0,   327,   282,   281,     0,   292,   293,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   384,   367,   386,   251,     0,   319,     0,     0,
     318,     0,     0,     0,     0,     0,     0,   404,   267,   268,
     284,   283,     0,   132,     0,     0,     0,     0,     0,   308,
       0,     0,     0,     0,   260,   262,   261,   391,   389,   390,
     164,   163,     0,   185,   186,     0,     0,     0,     0,    97,
       0,     0,     0,   276,   127,     0,     0,     0,     0,   136,
       0,     0,     0,     0,     0,   362,   361,   363,   366,     0,
     397,   399,   396,   398,   400,   401,     0,     0,     0,   104,
     105,   100,    98,     0,     0,     0,     0,    27,   151,    25,
       0,     0,     0,     0,     0,   316,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   228,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   368,   369,     0,     0,     0,
     377,     0,     0,     0,   383,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   133,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   158,   157,     0,     0,   152,     0,     0,     0,   359,
     345,   353,     0,     0,   348,   349,   350,   351,   352,     0,
     130,     0,   343,     0,     0,     0,     0,   121,   119,   125,
     123,     0,     0,     0,   155,     0,     0,   342,    12,   263,
       0,    11,     0,     0,   315,     0,     0,     0,   199,   198,
     173,   174,   195,     0,     0,     0,   227,     0,     0,   202,
     204,   246,   184,     0,   248,     0,     0,   189,     0,     0,
       0,     0,   325,     0,     0,   323,     0,     0,   322,     0,
       0,   385,   387,     0,     0,   294,   295,     0,   298,     0,
     296,     0,   297,   252,     0,     0,   253,     0,   176,     0,
       0,     0,     0,     0,   258,   257,     0,     0,   165,   166,
     277,   402,   403,     0,   178,     0,     0,     0,     0,     0,
     266,     0,     0,   148,     0,     0,   138,   275,   274,     0,
     357,   360,     0,   347,   135,   364,    99,     0,     0,   108,
       0,   107,     0,   106,     0,   112,     0,   103,     0,   102,
       0,   101,    29,   307,     0,     0,   317,   310,     0,   312,
       0,     0,   336,   394,   392,   393,   240,   237,   231,     0,
       0,   236,     0,   241,     0,   243,   244,     0,   239,   230,
     245,   395,   233,     0,   329,   203,     0,     0,   370,     0,
       0,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,     0,     0,
       0,   168,     0,     0,     0,   256,     0,     0,     0,     0,
       0,     0,     0,     0,   153,   147,   149,     0,     0,     0,
     128,     0,   120,   122,   124,   126,     0,   113,     0,   115,
       0,   117,     0,     0,   313,   194,   339,     0,     0,     0,
       0,     0,   332,     0,   247,    19,     0,   190,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   285,     0,
     303,   302,   273,     0,     0,   254,     0,   180,     0,     0,
     255,   259,     0,     0,   278,     0,   182,     0,   285,   188,
       0,   187,   160,     0,   140,   354,   355,   356,   358,     0,
       0,   111,     0,   110,     0,   109,     0,     0,   234,   235,
     238,   242,   232,     0,   299,   207,   208,     0,   212,   211,
     213,   214,   215,     0,     0,     0,   219,   220,     0,   205,
     209,   300,   206,     0,   249,   371,   373,     0,   378,     0,
     374,     0,   324,   376,   375,     0,     0,     0,   269,   304,
       0,     0,     0,     0,     0,     0,   191,   192,     0,     0,
       0,   169,     0,     0,     0,     0,     0,   140,   129,   114,
     116,   118,   264,     0,     0,   210,     0,     0,     0,     0,
      20,     0,     0,   326,     0,     0,   289,   290,   291,   286,
     287,   271,     0,     0,   175,     0,   285,   279,   167,   177,
       0,     0,   183,   265,     0,   144,   139,   141,     0,   301,
     216,   217,   218,   223,   222,   221,   379,     0,   380,   349,
       0,     0,   272,   270,     0,     0,     0,   171,     0,   170,
     142,    27,     0,     0,     0,     0,     0,   321,   288,     0,
     406,   179,     0,   181,    27,   145,     0,   224,   381,    16,
       0,   405,   172,   143,     0,     0,     0,    17,    21,    22,
      19,     0,     0,    14,    18,   382
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -654,  -654,   994,  -654,  -654,  -654,  -654,  -654,  -654,   146,
    -654,  -654,   815,   -83,  -290,   668,   848,   946,  -390,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,   959,  -654,  -654,  -654,   244,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,   614,
     849,  -654,  -654,  -654,  -654,   562,  -654,  -654,  -654,   260,
    -654,  -654,  -654,  -531,   253,  -654,   338,   223,  -654,  -654,
    -654,  -654,  -654,   187,  -654,  -654,   880,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,  -654,
    -654,  -654,  -654,  -654,  -654,  -654,  -654,   421,  -653,   200,
    -654,  -385,  -492,  -654,  -654,  -654,   369,   682,  -168,  -136,
    -312,   583,   150,  -308,  -386,  -485,  -418,  -473,   602,  -459,
    -132,   -55,  -654,   396,  -654,  -654,   610,  -654,  -654,   778,
    -135,   575,  -392,  -654,  -654,  -654,  -654,  -654,  -124,  -654
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,   191,    81,   836,   862,   734,
     860,    12,   134,    82,   338,    83,    84,    85,    86,   173,
      87,    88,   636,   638,   640,   423,   424,   425,   426,    89,
     409,   699,    90,    91,   389,    92,    93,   174,   627,   766,
     767,   844,   831,    94,   525,    95,    96,   188,    97,   522,
      98,   336,    99,   296,   402,   518,   100,   101,   102,   281,
     272,   103,   801,   104,   842,   352,   500,   516,   679,   688,
     105,   295,   690,   468,   758,   106,   210,   450,   107,   359,
     229,   585,   729,   815,   108,   356,   219,   355,   579,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   615,
     119,   120,   121,   441,   122,   123,   124,   125,   126,   791,
     823,   127,   128,   129,   130,   131,   234,   273,   748,   789,
     790,   237,   487,   491,   730,   672,   492,   196,   278,   253,
     212,   346,   259,   260,   477,   478,   230,   231,   220,   221,
     315,   279,   697,   530,   531,   532,   317,   318,   319,   254,
     376,   183,   291,   582,   333,   334,   335,   513,   266,   267
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     185,   211,   300,   197,   509,   182,   401,   238,   239,   611,
     255,   261,   262,   265,   549,   517,   427,   520,   521,   529,
     428,   527,   177,   283,   213,   284,   285,   286,   223,   275,
     276,   277,   297,   177,    16,   761,   177,   305,   320,   303,
     321,   202,   203,   263,   322,   323,   324,   551,   232,   439,
     329,   202,   203,   270,   331,   472,   293,   472,   213,   657,
     177,   213,   189,   813,   422,   641,   456,   498,   305,   459,
     177,   514,   677,   858,   686,   639,   256,   202,   203,   538,
     214,   540,   674,     9,   224,   644,   539,   637,   541,   756,
     691,   202,   203,   287,    11,   814,   325,   288,   289,   257,
     258,   193,   667,   282,   206,   644,   367,   368,   193,   370,
     294,   235,   194,   195,   214,   207,   236,   214,   190,   194,
     195,   132,   485,   306,    13,   207,   486,   859,   233,   316,
     223,   332,   274,   271,   407,   542,   208,   209,   326,     7,
     206,   499,   543,   827,   757,   515,   678,   223,   687,   670,
     706,   327,     1,     2,   215,   328,   671,   193,   225,     8,
     216,   472,   178,    60,   226,   327,   179,   180,   194,   195,
     741,   193,   208,   209,   257,   258,    14,   179,   180,   440,
     179,   180,   194,   195,   133,   519,   224,   519,   215,   178,
     176,   215,   546,   178,   216,   193,   429,   216,   511,   178,
     512,   654,   181,   224,   179,   180,   194,   195,   179,   180,
     475,   476,   445,   181,   179,   180,   181,   217,   218,   771,
     217,   218,   280,    15,   460,   461,   506,   462,   633,   770,
     632,   135,   624,   712,   469,   470,   471,   693,   479,   529,
     181,   769,   482,   523,   181,   488,   406,   489,   490,   764,
     181,   765,   412,   413,   435,   414,   415,   416,   417,   418,
     225,   510,   497,   136,   548,   137,   226,   503,   504,   505,
     563,   544,   306,   430,   564,   565,   528,   225,   545,   448,
     449,   774,   700,   226,   604,   436,   536,   227,   228,   701,
     702,   519,   263,   138,   447,   550,   139,   703,   140,   454,
     268,   269,   704,   458,   227,   228,   141,   202,   203,   705,
     472,   308,   467,   689,   473,   142,   475,   476,   298,   580,
     566,   567,   568,   287,   484,   202,   203,   288,   289,   588,
     589,   143,   692,   287,   309,   569,   330,   288,   289,   397,
     170,   310,   311,   312,   313,   314,   144,   524,   835,   570,
     571,   -15,   204,   205,   854,   312,   313,   314,   572,   573,
     574,   575,   576,   145,   316,   146,   202,   203,   695,   696,
     206,   147,   804,   148,   577,   149,   578,   150,   290,   151,
     186,   264,   152,   310,   311,   312,   313,   314,   206,   737,
     365,   366,   738,   310,   311,   312,   313,   314,   184,   207,
     153,   154,   208,   209,   312,   313,   314,   155,   156,   768,
     202,   203,   635,   157,   204,   205,   198,   199,   200,   201,
     208,   209,   240,   241,   242,   243,   244,   158,   245,   206,
     246,   247,   248,   249,   250,   817,   280,   159,   818,   192,
     474,   596,   348,   310,   311,   312,   313,   314,   682,   160,
     181,   786,   787,   788,   161,   162,   658,   163,   164,   165,
     661,   208,   209,   664,   665,   475,   476,   533,   739,   301,
     626,   252,   167,   206,   302,   168,   411,   169,   175,   307,
     709,   710,   711,   534,   207,   187,   684,   634,   304,   310,
     311,   312,   313,   314,   553,   340,   337,   202,   203,   342,
     341,   822,   343,   344,   345,   208,   209,   348,   347,   353,
     350,   829,   348,   349,   351,   708,   310,   311,   312,   313,
     314,   732,   310,   311,   312,   313,   314,   310,   311,   312,
     313,   314,   310,   311,   312,   313,   314,   354,   659,   660,
     357,  -201,   663,   310,   311,   312,   313,   314,   358,   360,
     369,   361,   852,   240,   241,   242,   243,   244,   362,   245,
     206,   246,   247,   248,   249,   250,   559,   202,   203,   371,
     363,   251,   310,   311,   312,   313,   314,   364,   372,   467,
     202,   203,   373,   375,   377,   775,   378,   797,   379,   380,
     381,   382,   208,   209,   310,   784,   312,   313,   314,   383,
     562,   735,   252,   384,   385,   386,   387,   388,   810,   811,
     812,   745,   390,   391,   392,   310,   311,   312,   313,   314,
     393,   394,   395,   240,   241,   242,   243,   244,   584,   245,
     206,   246,   247,   248,   249,   250,   240,   241,   242,   243,
     244,   251,   245,   206,   246,   247,   248,   249,   250,   310,
     311,   312,   313,   314,   251,   713,   714,   396,   602,   405,
     398,   399,   208,   209,   742,   400,   715,   716,   717,   403,
     404,   408,   252,   419,   438,   208,   209,   310,   311,   312,
     313,   314,   410,   420,   782,   252,   421,   431,   432,   718,
     433,   719,   720,   721,   722,   723,   724,   725,   726,   727,
     728,   442,   443,   444,   446,   451,   452,   310,   311,   312,
     313,   314,   453,   310,   311,   312,   313,   314,   455,   857,
     457,   463,   464,   465,   466,   864,   480,   493,   483,   819,
     481,   202,   203,   193,   494,   495,   496,   501,   502,    16,
     507,   526,  -159,   537,   194,   195,   554,    17,   845,   552,
      18,    19,    20,    21,    22,    23,    24,    25,    26,   555,
     557,   853,    27,    28,    29,   558,   561,   586,    30,    31,
      32,    33,    34,    35,    36,    37,    38,   583,    39,    40,
      41,    42,    43,    44,    45,   587,    46,   240,   241,   242,
     243,   244,   590,   245,   206,   246,   247,   248,   249,   250,
     592,    47,   597,   601,   591,   251,   593,   594,    48,    49,
     595,    50,   598,    51,    52,    53,   599,   600,    54,    55,
     603,    56,   604,    57,   605,   606,   208,   209,   607,   609,
     608,   612,   610,   613,   614,   616,   252,   617,   618,    62,
     619,   620,    58,   621,    59,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,   622,   596,   623,   630,   628,    60,   625,
     629,   631,   642,   256,    61,    62,   643,   645,   644,   646,
     647,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,   171,
      64,   648,    66,   649,    68,    63,    70,    65,    72,    67,
      74,    69,    76,    71,    78,    73,    80,    75,   652,    77,
     650,    79,   651,   653,   655,   656,   467,   662,   666,   667,
     669,   673,   675,   676,   674,   683,   680,   681,   685,   707,
     733,   746,   694,   751,   736,   740,   743,   744,   747,   750,
     749,   752,   754,   755,   759,   760,   781,   762,   773,   763,
     780,   803,   776,   772,   777,   778,   779,   792,   785,   783,
     793,   795,   794,   809,   800,   798,   799,   805,   808,   820,
     816,   821,   806,   824,   825,   828,   830,   832,   834,   837,
     839,   840,   833,   841,   843,   846,   848,   850,   849,    10,
     851,   172,   855,   856,   434,   861,   863,   339,   166,   865,
     508,   807,   560,   292,   796,   802,   753,   299,   826,   222,
     847,   838,   731,   437,   668,   698,   556,   374,     0,   535,
     581,     0,     0,   547
};

static const yytype_int16 yycheck[] =
{
      83,   137,   170,   135,   396,    60,   296,   143,   144,   501,
     145,   147,   148,   149,   432,   400,   328,   403,   404,   409,
     328,    59,     4,   159,     3,   160,   161,   162,     3,   153,
     154,   155,   167,     4,     6,   688,     4,     8,    20,   175,
      22,    59,    60,    44,    26,    27,    28,   433,    59,     3,
     186,    59,    60,    59,   186,     3,     5,     3,     3,   590,
       4,     3,     3,   107,     4,   550,   356,    59,     8,   359,
       4,    59,    59,     5,    59,   548,   133,    59,    60,   131,
      59,   131,   131,     0,    59,   134,   138,   546,   138,     4,
     621,    59,    60,     4,    70,   139,    78,     8,     9,   156,
     157,   139,   131,   158,   122,   134,   242,   243,   139,   245,
      59,    54,   150,   151,    59,   133,    59,    59,    59,   150,
     151,   131,    55,   178,   139,   133,    59,    59,   139,   184,
       3,   186,   133,   139,   302,   131,   154,   155,   120,   132,
     122,   133,   138,   796,    59,   133,   133,     3,   133,    59,
     642,   133,    15,    16,   133,   137,    66,   139,   133,   132,
     139,     3,   133,   135,   139,   133,   148,   149,   150,   151,
     662,   139,   154,   155,   156,   157,   139,   148,   149,   133,
     148,   149,   150,   151,    71,   133,    59,   133,   133,   133,
     141,   133,   137,   133,   139,   139,   328,   139,    57,   133,
      59,   586,   184,    59,   148,   149,   150,   151,   148,   149,
     158,   159,   347,   184,   148,   149,   184,   162,   163,   704,
     162,   163,   133,   132,   360,   361,   394,   362,   540,   702,
     538,   132,   522,   651,   369,   371,   372,   623,   373,   629,
     184,   700,   377,    87,   184,    59,   301,    61,    62,    89,
     184,    91,   307,   308,   337,   310,   311,   312,   313,   314,
     133,   397,   386,   132,   137,   132,   139,   391,   392,   393,
       4,   131,   327,   328,     8,     9,   408,   133,   138,    64,
      65,   132,   131,   139,   135,   340,   421,   160,   161,   138,
     131,   133,    44,   132,   349,   137,   132,   138,   132,   354,
     150,   151,   131,   358,   160,   161,   132,    59,    60,   138,
       3,   113,    58,    59,     7,   132,   158,   159,   168,   455,
      54,    55,    56,     4,   379,    59,    60,     8,     9,   465,
     466,   132,   622,     4,   136,    69,   186,     8,     9,   131,
     135,   185,   186,   187,   188,   189,   132,   191,   131,    83,
      84,   134,    63,    64,   846,   187,   188,   189,    92,    93,
      94,    95,    96,   132,   419,   132,    59,    60,   144,   145,
     122,   132,   764,   132,   108,   132,   110,   132,    59,   132,
     190,   133,   132,   185,   186,   187,   188,   189,   122,   131,
     240,   241,   134,   185,   186,   187,   188,   189,   133,   133,
     132,   132,   154,   155,   187,   188,   189,   132,   132,   699,
      59,    60,   544,   132,    63,    64,    10,    11,    12,    13,
     154,   155,   115,   116,   117,   118,   119,   132,   121,   122,
     123,   124,   125,   126,   127,   131,   133,   132,   134,   131,
     133,     3,     4,   185,   186,   187,   188,   189,   616,   132,
     184,    32,    33,    34,   132,   132,   591,   132,   132,   132,
     595,   154,   155,   598,   599,   158,   159,   136,   131,   190,
     525,   164,   132,   122,   135,   132,   134,   132,   132,   135,
     648,   649,   650,   136,   133,   132,   618,   542,   133,   185,
     186,   187,   188,   189,   136,   135,   137,    59,    60,   131,
     191,   791,   131,   131,   130,   154,   155,     4,   133,     3,
     131,   801,     4,   135,   131,   647,   185,   186,   187,   188,
     189,   653,   185,   186,   187,   188,   189,   185,   186,   187,
     188,   189,   185,   186,   187,   188,   189,   135,   593,   594,
       3,   137,   597,   185,   186,   187,   188,   189,   135,   131,
     133,   131,   842,   115,   116,   117,   118,   119,   131,   121,
     122,   123,   124,   125,   126,   127,   136,    59,    60,   133,
     131,   133,   185,   186,   187,   188,   189,   131,   133,    58,
      59,    60,   133,   133,   193,   717,     4,   755,   135,   131,
     131,   131,   154,   155,   185,   186,   187,   188,   189,   133,
     136,   656,   164,     4,   131,   131,   131,     6,   776,   777,
     778,   666,     4,   131,   131,   185,   186,   187,   188,   189,
     131,   131,     4,   115,   116,   117,   118,   119,   136,   121,
     122,   123,   124,   125,   126,   127,   115,   116,   117,   118,
     119,   133,   121,   122,   123,   124,   125,   126,   127,   185,
     186,   187,   188,   189,   133,    61,    62,   131,   136,   136,
     131,   131,   154,   155,   136,   131,    72,    73,    74,   131,
     131,   131,   164,   131,     3,   154,   155,   185,   186,   187,
     188,   189,   134,   134,   739,   164,   132,   132,   132,    95,
     132,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,     3,   131,   136,   131,     5,   140,   185,   186,   187,
     188,   189,   131,   185,   186,   187,   188,   189,   131,   855,
     131,    58,    58,   131,   131,   861,   128,    67,   131,   784,
     134,    59,    60,   139,     4,   131,    67,   131,   131,     6,
     131,   136,    77,   131,   150,   151,     3,    14,   831,   138,
      17,    18,    19,    20,    21,    22,    23,    24,    25,   131,
     134,   844,    29,    30,    31,     4,   139,   131,    35,    36,
      37,    38,    39,    40,    41,    42,    43,   139,    45,    46,
      47,    48,    49,    50,    51,   131,    53,   115,   116,   117,
     118,   119,   192,   121,   122,   123,   124,   125,   126,   127,
     134,    68,   135,     4,   131,   133,   131,   131,    75,    76,
     131,    78,   131,    80,    81,    82,   131,   131,    85,    86,
     131,    88,   135,    90,   131,   131,   154,   155,     4,     4,
     131,     4,   131,   131,   131,   131,   164,    58,   131,   142,
       4,   131,   109,   131,   111,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    77,     3,   131,   134,   132,   135,   191,
     131,     4,   131,   133,   141,   142,     3,   140,   134,   134,
     132,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   142,
     149,   132,   151,   132,   153,   148,   155,   150,   157,   152,
     159,   154,   161,   156,   163,   158,   165,   160,   134,   162,
     132,   164,   132,   131,     5,   131,    58,   131,   133,   131,
       4,     4,    67,   131,   131,   131,   139,    67,   131,   131,
     131,     4,   137,     4,   134,   134,   134,   134,   131,   131,
     136,     4,   131,   131,     4,     4,    32,   131,     5,   131,
       3,     7,   132,   134,   132,   132,   132,     4,   131,   134,
     131,   131,   134,    59,   131,   134,   134,   132,   131,     4,
     134,   131,   138,   131,     4,     4,   132,     5,    32,   134,
       4,   134,   192,   134,   134,   131,   134,   134,   129,     5,
     134,    55,   131,   131,   336,   131,   860,   192,    49,   134,
     396,   767,   450,   164,   754,   762,   678,   169,   795,   139,
     833,   821,   653,   341,   603,   629,   443,   249,    -1,   419,
     455,    -1,    -1,   431
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    15,    16,   195,   196,   197,   198,   132,   132,     0,
     196,    70,   205,   139,   139,   132,     6,    14,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    29,    30,    31,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    45,
      46,    47,    48,    49,    50,    51,    53,    68,    75,    76,
      78,    80,    81,    82,    85,    86,    88,    90,   109,   111,
     135,   141,   142,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   200,   207,   209,   210,   211,   212,   214,   215,   223,
     226,   227,   229,   230,   237,   239,   240,   242,   244,   246,
     250,   251,   252,   255,   257,   264,   269,   272,   278,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   294,
     295,   296,   298,   299,   300,   301,   302,   305,   306,   307,
     308,   309,   131,    71,   206,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   229,   132,   132,   132,
     135,   142,   211,   213,   231,   132,   141,     4,   133,   148,
     149,   184,   335,   345,   133,   207,   190,   132,   241,     3,
      59,   199,   131,   139,   150,   151,   321,   334,    10,    11,
      12,    13,    59,    60,    63,    64,   122,   133,   154,   155,
     270,   323,   324,     3,    59,   133,   139,   162,   163,   280,
     332,   333,   280,     3,    59,   133,   139,   160,   161,   274,
     330,   331,    59,   139,   310,    54,    59,   315,   323,   323,
     115,   116,   117,   118,   119,   121,   123,   124,   125,   126,
     127,   133,   164,   323,   343,   344,   133,   156,   157,   326,
     327,   323,   323,    44,   133,   323,   352,   353,   326,   326,
      59,   139,   254,   311,   133,   352,   352,   352,   322,   335,
     133,   253,   335,   323,   344,   344,   344,     4,     8,     9,
      59,   346,   254,     5,    59,   265,   247,   344,   326,   210,
     322,   190,   135,   323,   133,     8,   335,   135,   113,   136,
     185,   186,   187,   188,   189,   334,   335,   340,   341,   342,
      20,    22,    26,    27,    28,    78,   120,   133,   137,   323,
     326,   334,   335,   348,   349,   350,   245,   137,   208,   206,
     135,   191,   131,   131,   131,   130,   325,   133,     4,   135,
     131,   131,   259,     3,   135,   281,   279,     3,   135,   273,
     131,   131,   131,   131,   131,   326,   326,   323,   323,   133,
     323,   133,   133,   133,   343,   133,   344,   193,     4,   135,
     131,   131,   131,   133,     4,   131,   131,   131,     6,   228,
       4,   131,   131,   131,   131,     4,   131,   131,   131,   131,
     131,   208,   248,   131,   131,   136,   335,   322,   131,   224,
     134,   134,   335,   335,   335,   335,   335,   335,   335,   131,
     134,   132,     4,   219,   220,   221,   222,   324,   327,   334,
     335,   132,   132,   132,   209,   207,   335,   321,     3,     3,
     133,   297,     3,   131,   136,   344,   131,   335,    64,    65,
     271,     5,   140,   131,   335,   131,   208,   131,   335,   208,
     323,   323,   344,    58,    58,   131,   131,    58,   267,   344,
     323,   323,     3,     7,   133,   158,   159,   328,   329,   344,
     128,   134,   344,   131,   335,    55,    59,   316,    59,    61,
      62,   317,   320,    67,     4,   131,    67,   352,    59,   133,
     260,   131,   131,   352,   352,   352,   322,   131,   253,   346,
     323,    57,    59,   351,    59,   133,   261,   315,   249,   133,
     328,   328,   243,    87,   191,   238,   136,    59,   334,   212,
     337,   338,   339,   136,   136,   340,   344,   131,   131,   138,
     131,   138,   131,   138,   131,   138,   137,   332,   137,   330,
     137,   328,   138,   136,     3,   131,   325,   134,     4,   136,
     259,   139,   136,     4,     8,     9,    54,    55,    56,    69,
      83,    84,    92,    93,    94,    95,    96,   108,   110,   282,
     323,   345,   347,   139,   136,   275,   131,   131,   323,   323,
     192,   131,   134,   131,   131,   131,     3,   135,   131,   131,
     131,     4,   136,   131,   135,   131,   131,     4,   131,     4,
     131,   316,     4,   131,   131,   293,   131,    58,   131,     4,
     131,   131,    77,   131,   208,   191,   335,   232,   132,   131,
     134,     4,   327,   324,   335,   334,   216,   333,   217,   331,
     218,   329,   131,     3,   134,   140,   134,   132,   132,   132,
     132,   132,   134,   131,   315,     5,   131,   267,   344,   335,
     335,   344,   131,   335,   344,   344,   133,   131,   311,     4,
      59,    66,   319,     4,   131,    67,   131,    59,   133,   262,
     139,    67,   322,   131,   334,   131,    59,   133,   263,    59,
     266,   267,   208,   328,   137,   144,   145,   336,   337,   225,
     131,   138,   131,   138,   131,   138,   316,   131,   334,   322,
     322,   322,   330,    61,    62,    72,    73,    74,    95,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   276,
     318,   320,   334,   131,   203,   335,   134,   131,   134,   131,
     134,   316,   136,   134,   134,   335,     4,   131,   312,   136,
     131,     4,     4,   270,   131,   131,     4,    59,   268,     4,
       4,   312,   131,   131,    89,    91,   233,   234,   208,   333,
     331,   329,   134,     5,   132,   334,   132,   132,   132,   132,
       3,    32,   335,   134,   186,   131,    32,    33,    34,   313,
     314,   303,     4,   131,   134,   131,   263,   322,   134,   134,
     131,   256,   268,     7,   346,   132,   138,   233,   131,    59,
     322,   322,   322,   107,   139,   277,   134,   131,   134,   335,
       4,   131,   208,   304,   131,     4,   271,   312,     4,   208,
     132,   236,     5,   192,    32,   131,   201,   134,   313,     4,
     134,   134,   258,   134,   235,   207,   131,   277,   134,   129,
     134,   134,   208,   207,   316,   131,   131,   323,     5,    59,
     204,   131,   202,   203,   323,   134
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,   194,   195,   195,   196,   196,   197,   198,   198,   199,
     199,   200,   200,   200,   200,   201,   201,   202,   202,   203,
     203,   204,   204,   205,   205,   206,   206,   207,   207,   208,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   209,   209,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   212,   212,   212,   213,   213,   214,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   216,   216,   217,   217,   218,   218,   219,
     219,   220,   220,   221,   221,   222,   222,   224,   225,   223,
     226,   227,   228,   228,   229,   229,   229,   231,   232,   230,
     233,   233,   235,   234,   236,   234,   237,   238,   238,   239,
     241,   240,   243,   242,   245,   244,   247,   246,   248,   249,
     248,   250,   251,   251,   251,   252,   252,   253,   254,   256,
     255,   258,   257,   259,   259,   260,   260,   261,   261,   262,
     262,   263,   263,   264,   264,   265,   265,   266,   266,   267,
     267,   268,   268,   269,   269,   269,   270,   270,   271,   271,
     272,   273,   272,   274,   275,   275,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   277,   277,   277,   278,   279,   278,   280,   281,
     281,   282,   282,   282,   282,   282,   282,   282,   282,   282,
     282,   282,   282,   282,   282,   282,   283,   284,   285,   285,
     286,   286,   287,   288,   289,   290,   291,   292,   293,   293,
     294,   295,   296,   297,   297,   298,   299,   300,   301,   303,
     302,   304,   304,   305,   306,   306,   306,   307,   308,   309,
     309,   310,   310,   311,   311,   312,   312,   313,   313,   314,
     314,   314,   315,   315,   316,   316,   317,   317,   317,   318,
     318,   318,   319,   319,   320,   321,   321,   321,   322,   323,
     323,   323,   323,   324,   324,   324,   325,   325,   326,   326,
     326,   327,   328,   328,   328,   329,   329,   330,   330,   330,
     331,   331,   331,   331,   332,   332,   332,   333,   333,   333,
     333,   334,   334,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   336,   336,   337,   338,   338,   339,
     339,   340,   340,   341,   341,   342,   342,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   344,   344,   345,   346,
     346,   346,   347,   347,   347,   347,   348,   348,   349,   349,
     350,   350,   351,   351,   352,   352,   353
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     3,     3,     5,     1,
       1,     5,     5,     3,    16,     0,     2,     0,     2,     0,
       2,     1,     1,     0,     3,     3,     1,     0,     2,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     5,
       3,     5,     5,     5,     3,     3,     5,     5,     5,     7,
       7,     7,     5,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     0,     0,     8,
       4,     1,     0,     1,     1,     5,     3,     0,     0,     9,
       0,     2,     0,     5,     0,     4,     1,     2,     1,     6,
       0,     3,     0,     6,     0,     4,     0,     4,     1,     0,
       4,     3,     1,     3,     3,     5,     5,     7,     4,     0,
      10,     0,    12,     0,     2,     5,     1,     5,     1,     5,
       1,     5,     1,     9,     5,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     7,     5,     1,     1,     1,     1,
       3,     0,     5,     4,     0,     3,     1,     1,     1,     1,
       2,     1,     1,     1,     1,     1,     3,     3,     3,     1,
       1,     3,     1,     1,     3,     3,     0,     5,     2,     0,
       3,     1,     3,     1,     3,     3,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     1,     5,     7,     5,     8,
       1,     3,     5,     5,     7,     7,     6,     5,     0,     2,
       3,     3,     3,     1,     5,     9,     5,     3,     3,     0,
      10,     0,     1,     7,     5,     5,     3,     5,     7,     9,
       1,     1,     1,     1,     1,     0,     2,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     4,     1,     1,     4,     1,     1,
       4,     1,     4,     5,     1,     3,     1,     3,     1,     1,
       4,     9,     1,     1,     4,     1,     5,     1,     1,     4,
       1,     1,     5,     1,     1,     1,     4,     1,     1,     5,
       1,     1,     3,     1,     1,     3,     1,     4,     3,     3,
       3,     3,     3,     3,     1,     1,     3,     1,     3,     0,
       1,     1,     1,     1,     3,     0,     1,     1,     2,     2,
       4,     6,     4,     6,     6,     6,     6,     2,     6,     8,
       8,    10,    14,     2,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,    10,     9
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
  case 6: /* level: level_def flags levstatements  */
#line 290 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			if (fatal_error > 0) {
				(void) fprintf(stderr,
              "%s: %d errors detected for level \"%s\". No output created!\n",
					       fname, fatal_error, (yyvsp[-2].map));
				fatal_error = 0;
				got_errors++;
			} else if (!got_errors) {
				if (!write_level_file((yyvsp[-2].map), splev)) {
                                    lc_error("Can't write output file for '%s'!",
                                             (yyvsp[-2].map));
				    exit(EXIT_FAILURE);
				}
			}
			Free((yyvsp[-2].map));
			Free(splev);
			splev = NULL;
			vardef_free_all(vardefs);
			vardefs = NULL;
		  }
#line 2287 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 7: /* level_def: LEVEL_ID ':' STRING  */
#line 313 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      start_level_def(&splev, (yyvsp[0].map));
		      (yyval.map) = (yyvsp[0].map);
		  }
#line 2296 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 8: /* level_def: MAZE_ID ':' STRING ',' mazefiller  */
#line 318 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      start_level_def(&splev, (yyvsp[-2].map));
		      if ((yyvsp[0].i) == -1) {
			  add_opvars(splev, "iiiiiiiio",
				     VA_PASS9(LVLINIT_MAZEGRID, HWALL, 0,0,
					      0,0,0,0, SPO_INITLEVEL));
		      } else {
			  int bg = (int) what_map_char((char) (yyvsp[0].i));

			  add_opvars(splev, "iiiiiiiio",
				     VA_PASS9(LVLINIT_SOLIDFILL, bg, 0,0,
					      0,0,0,0, SPO_INITLEVEL));
		      }
		      add_opvars(splev, "io",
				 VA_PASS2(MAZELEVEL, SPO_LEVEL_FLAGS));
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		      (yyval.map) = (yyvsp[-2].map);
		  }
#line 2320 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 9: /* mazefiller: RANDOM_TYPE  */
#line 340 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = -1;
		  }
#line 2328 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 10: /* mazefiller: CHAR  */
#line 344 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = what_map_char((char) (yyvsp[0].i));
		  }
#line 2336 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 11: /* lev_init: LEV_INIT_ID ':' SOLID_FILL_ID ',' terrain_type  */
#line 350 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int filling = (int) (yyvsp[0].terr).ter;

		      if (filling == INVALID_TYPE || filling >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");
		      add_opvars(splev, "iiiiiiiio",
				 VA_PASS9(LVLINIT_SOLIDFILL, filling,
                                          0, (int) (yyvsp[0].terr).lit,
                                          0,0,0,0, SPO_INITLEVEL));
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
#line 2353 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 12: /* lev_init: LEV_INIT_ID ':' MAZE_GRID_ID ',' CHAR  */
#line 363 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int filling = (int) what_map_char((char) (yyvsp[0].i));

		      if (filling == INVALID_TYPE || filling >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");
                      add_opvars(splev, "iiiiiiiio",
				 VA_PASS9(LVLINIT_MAZEGRID, filling, 0,0,
					  0,0,0,0, SPO_INITLEVEL));
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
#line 2369 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 13: /* lev_init: LEV_INIT_ID ':' ROGUELEV_ID  */
#line 375 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiiiiiio",
				 VA_PASS9(LVLINIT_ROGUE,0,0,0,
					  0,0,0,0, SPO_INITLEVEL));
		  }
#line 2379 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 14: /* lev_init: LEV_INIT_ID ':' MINES_ID ',' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled opt_fillchar  */
#line 381 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      int fg = (int) what_map_char((char) (yyvsp[-11].i)),
                          bg = (int) what_map_char((char) (yyvsp[-9].i));
                      int smoothed = (int) (yyvsp[-7].i),
                          joined = (int) (yyvsp[-5].i),
                          lit = (int) (yyvsp[-3].i),
                          walled = (int) (yyvsp[-1].i),
                          filling = (int) (yyvsp[0].i);

		      if (fg == INVALID_TYPE || fg >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid foreground type.");
		      if (bg == INVALID_TYPE || bg >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid background type.");
		      if (joined && fg != CORR && fg != ROOM)
			  lc_error("INIT_MAP: Invalid foreground type for joined map.");

		      if (filling == INVALID_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");

		      add_opvars(splev, "iiiiiiiio",
				 VA_PASS9(LVLINIT_MINES, filling, walled, lit,
					  joined, smoothed, bg, fg,
					  SPO_INITLEVEL));
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
#line 2410 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 15: /* opt_limited: %empty  */
#line 410 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 0;
		  }
#line 2418 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 16: /* opt_limited: ',' LIMITED  */
#line 414 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[0].i);
		  }
#line 2426 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 17: /* opt_coord_or_var: %empty  */
#line 420 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_COPY));
		      (yyval.i) = 0;
		  }
#line 2435 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 18: /* opt_coord_or_var: ',' coord_or_var  */
#line 425 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1;
		  }
#line 2443 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 19: /* opt_fillchar: %empty  */
#line 431 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = -1;
		  }
#line 2451 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 20: /* opt_fillchar: ',' CHAR  */
#line 435 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = what_map_char((char) (yyvsp[0].i));
		  }
#line 2459 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 23: /* flags: %empty  */
#line 446 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(0, SPO_LEVEL_FLAGS));
		  }
#line 2467 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 24: /* flags: FLAGS_ID ':' flag_list  */
#line 450 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
                                 VA_PASS2((int) (yyvsp[0].i), SPO_LEVEL_FLAGS));
		  }
#line 2476 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 25: /* flag_list: FLAG_TYPE ',' flag_list  */
#line 457 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = ((yyvsp[-2].i) | (yyvsp[0].i));
		  }
#line 2484 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 26: /* flag_list: FLAG_TYPE  */
#line 461 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[0].i);
		  }
#line 2492 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 27: /* levstatements: %empty  */
#line 467 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 0;
		  }
#line 2500 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 28: /* levstatements: levstatement levstatements  */
#line 471 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1 + (yyvsp[0].i);
		  }
#line 2508 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 29: /* stmt_block: '{' levstatements '}'  */
#line 477 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[-1].i);
		  }
#line 2516 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 97: /* shuffle_detail: SHUFFLE_ID ':' any_var_array  */
#line 560 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct lc_vardefs *vd;

		      if ((vd = vardef_defined(vardefs, (yyvsp[0].map), 1))) {
			  if (!(vd->var_type & SPOVAR_ARRAY))
			      lc_error("Trying to shuffle non-array variable '%s'",
                                       (yyvsp[0].map));
		      } else
                          lc_error("Trying to shuffle undefined variable '%s'",
                                   (yyvsp[0].map));
		      add_opvars(splev, "so", VA_PASS2((yyvsp[0].map), SPO_SHUFFLE_ARRAY));
		      Free((yyvsp[0].map));
		  }
#line 2534 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 98: /* variable_define: any_var_or_arr '=' math_expr_var  */
#line 576 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-2].map), SPOVAR_INT);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-2].map), SPO_VAR_INIT));
		      Free((yyvsp[-2].map));
		  }
#line 2544 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 99: /* variable_define: any_var_or_arr '=' selection_ID ':' ter_selection  */
#line 582 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map), SPOVAR_SEL);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2554 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 100: /* variable_define: any_var_or_arr '=' string_expr  */
#line 588 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-2].map), SPOVAR_STRING);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-2].map), SPO_VAR_INIT));
		      Free((yyvsp[-2].map));
		  }
#line 2564 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 101: /* variable_define: any_var_or_arr '=' terrainid ':' mapchar_or_var  */
#line 594 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map), SPOVAR_MAPCHAR);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2574 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 102: /* variable_define: any_var_or_arr '=' monsterid ':' monster_or_var  */
#line 600 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map), SPOVAR_MONST);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2584 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 103: /* variable_define: any_var_or_arr '=' objectid ':' object_or_var  */
#line 606 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map), SPOVAR_OBJ);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2594 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 104: /* variable_define: any_var_or_arr '=' coord_or_var  */
#line 612 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-2].map), SPOVAR_COORD);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-2].map), SPO_VAR_INIT));
		      Free((yyvsp[-2].map));
		  }
#line 2604 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 105: /* variable_define: any_var_or_arr '=' region_or_var  */
#line 618 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      vardefs = add_vardef_type(vardefs, (yyvsp[-2].map), SPOVAR_REGION);
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-2].map), SPO_VAR_INIT));
		      Free((yyvsp[-2].map));
		  }
#line 2614 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 106: /* variable_define: any_var_or_arr '=' '{' integer_list '}'  */
#line 624 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map),
                                                SPOVAR_INT | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2628 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 107: /* variable_define: any_var_or_arr '=' '{' encodecoord_list '}'  */
#line 634 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map),
                                                SPOVAR_COORD | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2642 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 108: /* variable_define: any_var_or_arr '=' '{' encoderegion_list '}'  */
#line 644 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map),
                                                SPOVAR_REGION | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2656 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 109: /* variable_define: any_var_or_arr '=' terrainid ':' '{' mapchar_list '}'  */
#line 654 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-6].map),
                                                SPOVAR_MAPCHAR | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-6].map), SPO_VAR_INIT));
		      Free((yyvsp[-6].map));
		  }
#line 2670 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 110: /* variable_define: any_var_or_arr '=' monsterid ':' '{' encodemonster_list '}'  */
#line 664 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-6].map),
                                                SPOVAR_MONST | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-6].map), SPO_VAR_INIT));
		      Free((yyvsp[-6].map));
		  }
#line 2684 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 111: /* variable_define: any_var_or_arr '=' objectid ':' '{' encodeobj_list '}'  */
#line 674 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-6].map),
                                                SPOVAR_OBJ | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-6].map), SPO_VAR_INIT));
		      Free((yyvsp[-6].map));
		  }
#line 2698 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 112: /* variable_define: any_var_or_arr '=' '{' string_list '}'  */
#line 684 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      int n_items = (int) (yyvsp[-1].i);

		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map),
                                                SPOVAR_STRING | SPOVAR_ARRAY);
		      add_opvars(splev, "iso",
				 VA_PASS3(n_items, (yyvsp[-4].map), SPO_VAR_INIT));
		      Free((yyvsp[-4].map));
		  }
#line 2712 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 113: /* encodeobj_list: encodeobj  */
#line 696 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "O", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1;
		  }
#line 2721 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 114: /* encodeobj_list: encodeobj_list ',' encodeobj  */
#line 701 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "O", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2730 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 115: /* encodemonster_list: encodemonster  */
#line 708 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "M", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1;
		  }
#line 2739 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 116: /* encodemonster_list: encodemonster_list ',' encodemonster  */
#line 713 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "M", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2748 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 117: /* mapchar_list: mapchar  */
#line 720 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "m", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1;
		  }
#line 2757 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 118: /* mapchar_list: mapchar_list ',' mapchar  */
#line 725 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "m", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2766 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 119: /* encoderegion_list: encoderegion  */
#line 732 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1;
		  }
#line 2774 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 120: /* encoderegion_list: encoderegion_list ',' encoderegion  */
#line 736 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2782 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 121: /* encodecoord_list: encodecoord  */
#line 742 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "c", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1;
		  }
#line 2791 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 122: /* encodecoord_list: encodecoord_list ',' encodecoord  */
#line 747 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "c", VA_PASS1((yyvsp[0].i)));
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2800 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 123: /* integer_list: math_expr_var  */
#line 754 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1;
		  }
#line 2808 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 124: /* integer_list: integer_list ',' math_expr_var  */
#line 758 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2816 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 125: /* string_list: string_expr  */
#line 764 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1;
		  }
#line 2824 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 126: /* string_list: string_list ',' string_expr  */
#line 768 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 1 + (yyvsp[-2].i);
		  }
#line 2832 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 127: /* $@1: %empty  */
#line 774 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct lc_funcdefs *funcdef;

		      if (in_function_definition)
			  lc_error("Recursively defined functions not allowed (function %s).", (yyvsp[-1].map));

		      in_function_definition++;

		      if (funcdef_defined(function_definitions, (yyvsp[-1].map), 1))
			  lc_error("Function '%s' already defined once.", (yyvsp[-1].map));

		      funcdef = funcdef_new(-1, (yyvsp[-1].map));
		      funcdef->next = function_definitions;
		      function_definitions = funcdef;
		      function_splev_backup = splev;
		      splev = &(funcdef->code);
		      Free((yyvsp[-1].map));
		      curr_function = funcdef;
		      function_tmp_var_defs = vardefs;
		      vardefs = NULL;
		  }
#line 2858 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 128: /* $@2: %empty  */
#line 796 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 2866 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 129: /* function_define: FUNCTION_ID NQSTRING '(' $@1 func_params_list ')' $@2 stmt_block  */
#line 800 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(0, SPO_RETURN));
		      splev = function_splev_backup;
		      in_function_definition--;
		      curr_function = NULL;
		      vardef_free_all(vardefs);
		      vardefs = function_tmp_var_defs;
		  }
#line 2879 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 130: /* function_call: NQSTRING '(' func_call_params_list ')'  */
#line 811 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct lc_funcdefs *tmpfunc;

		      tmpfunc = funcdef_defined(function_definitions, (yyvsp[-3].map), 1);
		      if (tmpfunc) {
			  int l;
			  int nparams = (int) strlen((yyvsp[-1].map));
			  char *fparamstr = funcdef_paramtypes(tmpfunc);

			  if (strcmp((yyvsp[-1].map), fparamstr)) {
			      char *tmps = strdup(decode_parm_str(fparamstr));

			      lc_error("Function '%s' requires params '%s', got '%s' instead.",
                                       (yyvsp[-3].map), tmps, decode_parm_str((yyvsp[-1].map)));
			      Free(tmps);
			  }
			  Free(fparamstr);
			  Free((yyvsp[-1].map));
			  if (!(tmpfunc->n_called)) {
			      /* we haven't called the function yet, so insert it in the code */
			      struct opvar *jmp = New(struct opvar);

			      set_opvar_int(jmp, splev->n_opcodes+1);
			      add_opcode(splev, SPO_PUSH, jmp);
                              /* we must jump past it first, then CALL it, due to RETURN. */
			      add_opcode(splev, SPO_JMP, NULL);

			      tmpfunc->addr = splev->n_opcodes;

			      { /* init function parameter variables */
				  struct lc_funcdefs_parm *tfp = tmpfunc->params;
				  while (tfp) {
				      add_opvars(splev, "iso",
						 VA_PASS3(0, tfp->name,
							  SPO_VAR_INIT));
				      tfp = tfp->next;
				  }
			      }

			      splev_add_from(splev, &(tmpfunc->code));
			      set_opvar_int(jmp,
                                            splev->n_opcodes - jmp->vardata.l);
			  }
			  l = (int) (tmpfunc->addr - splev->n_opcodes - 2);
			  add_opvars(splev, "iio",
				     VA_PASS3(nparams, l, SPO_CALL));
			  tmpfunc->n_called++;
		      } else {
			  lc_error("Function '%s' not defined.", (yyvsp[-3].map));
		      }
		      Free((yyvsp[-3].map));
		  }
#line 2936 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 131: /* exitstatement: EXIT_ID  */
#line 866 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opcode(splev, SPO_EXIT, NULL);
		  }
#line 2944 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 132: /* opt_percent: %empty  */
#line 872 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 100;
		  }
#line 2952 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 133: /* opt_percent: PERCENT  */
#line 876 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[0].i);
		  }
#line 2960 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 134: /* comparestmt: PERCENT  */
#line 882 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* val > rn2(100) */
		      add_opvars(splev, "iio",
				 VA_PASS3((int) (yyvsp[0].i), 100, SPO_RN2));
		      (yyval.i) = SPO_JG;
                  }
#line 2971 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 135: /* comparestmt: '[' math_expr_var COMPARE_TYPE math_expr_var ']'  */
#line 889 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[-2].i);
                  }
#line 2979 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 136: /* comparestmt: '[' math_expr_var ']'  */
#line 893 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* boolean, explicit foo != 0 */
		      add_opvars(splev, "i", VA_PASS1(0));
		      (yyval.i) = SPO_JNE;
                  }
#line 2989 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 137: /* $@3: %empty  */
#line 901 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      is_inconstant_number = 0;
		  }
#line 2997 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 138: /* $@4: %empty  */
#line 905 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *chkjmp;

		      if (in_switch_statement > 0)
			  lc_error("Cannot nest switch-statements.");

		      in_switch_statement++;

		      n_switch_case_list = 0;
		      switch_default_case = NULL;

		      if (!is_inconstant_number)
			  add_opvars(splev, "o", VA_PASS1(SPO_RN2));
		      is_inconstant_number = 0;

		      chkjmp = New(struct opvar);
		      set_opvar_int(chkjmp, splev->n_opcodes+1);
		      switch_check_jump = chkjmp;
		      add_opcode(splev, SPO_PUSH, chkjmp);
		      add_opcode(splev, SPO_JMP, NULL);
		      break_stmt_start();
		  }
#line 3024 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 139: /* switchstatement: SWITCH_ID $@3 '[' integer_or_var ']' $@4 '{' switchcases '}'  */
#line 928 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *endjump = New(struct opvar);
		      int i;

		      set_opvar_int(endjump, splev->n_opcodes+1);

		      add_opcode(splev, SPO_PUSH, endjump);
		      add_opcode(splev, SPO_JMP, NULL);

		      set_opvar_int(switch_check_jump,
			     splev->n_opcodes - switch_check_jump->vardata.l);

		      for (i = 0; i < n_switch_case_list; i++) {
			  add_opvars(splev, "oio",
				     VA_PASS3(SPO_COPY,
					      switch_case_value[i], SPO_CMP));
			  set_opvar_int(switch_case_list[i],
			 switch_case_list[i]->vardata.l - splev->n_opcodes-1);
			  add_opcode(splev, SPO_PUSH, switch_case_list[i]);
			  add_opcode(splev, SPO_JE, NULL);
		      }

		      if (switch_default_case) {
			  set_opvar_int(switch_default_case,
			 switch_default_case->vardata.l - splev->n_opcodes-1);
			  add_opcode(splev, SPO_PUSH, switch_default_case);
			  add_opcode(splev, SPO_JMP, NULL);
		      }

		      set_opvar_int(endjump, splev->n_opcodes - endjump->vardata.l);

		      break_stmt_end(splev);

		      add_opcode(splev, SPO_POP, NULL); /* get rid of the value in stack */
		      in_switch_statement--;


		  }
#line 3067 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 142: /* $@5: %empty  */
#line 973 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (n_switch_case_list < MAX_SWITCH_CASES) {
			  struct opvar *tmppush = New(struct opvar);

			  set_opvar_int(tmppush, splev->n_opcodes);
			  switch_case_value[n_switch_case_list] = (yyvsp[-1].i);
			  switch_case_list[n_switch_case_list++] = tmppush;
		      } else lc_error("Too many cases in a switch.");
		  }
#line 3081 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 143: /* switchcase: CASE_ID all_integers ':' $@5 levstatements  */
#line 983 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		  }
#line 3088 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 144: /* $@6: %empty  */
#line 986 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *tmppush = New(struct opvar);

		      if (switch_default_case)
			  lc_error("Switch default case already used.");

		      set_opvar_int(tmppush, splev->n_opcodes);
		      switch_default_case = tmppush;
		  }
#line 3102 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 145: /* switchcase: DEFAULT_ID ':' $@6 levstatements  */
#line 996 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		  }
#line 3109 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 146: /* breakstatement: BREAK_ID  */
#line 1001 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (!allow_break_statements)
			  lc_error("Cannot use BREAK outside a statement block.");
		      else {
			  break_stmt_new(splev, splev->n_opcodes);
		      }
		  }
#line 3121 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 149: /* forstmt_start: FOR_ID any_var_or_unk '=' math_expr_var for_to_span math_expr_var  */
#line 1015 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      char buf[256], buf2[256];

		      if (n_forloops >= MAX_NESTED_IFS) {
			  lc_error("FOR: Too deeply nested loops.");
			  n_forloops = MAX_NESTED_IFS - 1;
		      }

		      /* first, define a variable for the for-loop end value */
		      Sprintf(buf, "%s end", (yyvsp[-4].map));
		      /* the value of which is already in stack (the 2nd math_expr) */
		      add_opvars(splev, "iso", VA_PASS3(0, buf, SPO_VAR_INIT));

		      vardefs = add_vardef_type(vardefs, (yyvsp[-4].map), SPOVAR_INT);
		      /* define the for-loop variable. value is in stack (1st math_expr) */
		      add_opvars(splev, "iso", VA_PASS3(0, (yyvsp[-4].map), SPO_VAR_INIT));

		      /* calculate value for the loop "step" variable */
		      Sprintf(buf2, "%s step", (yyvsp[-4].map));
		      /* end - start */
		      add_opvars(splev, "vvo",
				 VA_PASS3(buf, (yyvsp[-4].map), SPO_MATH_SUB));
		      /* sign of that */
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_SIGN));
		      /* save the sign into the step var */
		      add_opvars(splev, "iso",
				 VA_PASS3(0, buf2, SPO_VAR_INIT));

		      forloop_list[n_forloops].varname = strdup((yyvsp[-4].map));
		      forloop_list[n_forloops].jmp_point = splev->n_opcodes;

		      n_forloops++;
		      Free((yyvsp[-4].map));
		  }
#line 3160 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 150: /* $@7: %empty  */
#line 1052 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		      break_stmt_start();
		  }
#line 3169 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 151: /* forstatement: forstmt_start $@7 stmt_block  */
#line 1057 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      int l;
		      char buf[256], buf2[256];

		      n_forloops--;
		      Sprintf(buf, "%s step", forloop_list[n_forloops].varname);
		      Sprintf(buf2, "%s end", forloop_list[n_forloops].varname);
		      /* compare for-loop var to end value */
		      add_opvars(splev, "vvo",
				 VA_PASS3(forloop_list[n_forloops].varname,
					  buf2, SPO_CMP));
		      /* var + step */
		      add_opvars(splev, "vvo",
				VA_PASS3(buf, forloop_list[n_forloops].varname,
					 SPO_MATH_ADD));
		      /* for-loop var = (for-loop var + step) */
		      add_opvars(splev, "iso",
				 VA_PASS3(0, forloop_list[n_forloops].varname,
					  SPO_VAR_INIT));
		      /* jump back if compared values were not equal */
                      l = (int) (forloop_list[n_forloops].jmp_point
                                 - splev->n_opcodes - 1);
		      add_opvars(splev, "io", VA_PASS2(l, SPO_JNE));
		      Free(forloop_list[n_forloops].varname);
		      break_stmt_end(splev);
		  }
#line 3200 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 152: /* $@8: %empty  */
#line 1086 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *tmppush = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("LOOP: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }
		      set_opvar_int(tmppush, splev->n_opcodes);
		      if_list[n_if_list++] = tmppush;

		      add_opvars(splev, "o", VA_PASS1(SPO_DEC));
		      break_stmt_start();
		  }
#line 3218 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 153: /* loopstatement: LOOP_ID '[' integer_or_var ']' $@8 stmt_block  */
#line 1100 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *tmppush;

		      add_opvars(splev, "oio", VA_PASS3(SPO_COPY, 0, SPO_CMP));

		      tmppush = (struct opvar *) if_list[--n_if_list];
		      set_opvar_int(tmppush,
                                    tmppush->vardata.l - splev->n_opcodes-1);
		      add_opcode(splev, SPO_PUSH, tmppush);
		      add_opcode(splev, SPO_JG, NULL);
		      add_opcode(splev, SPO_POP, NULL); /* discard count */
		      break_stmt_end(splev);
		  }
#line 3236 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 154: /* $@9: %empty  */
#line 1116 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *tmppush2 = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("IF: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }

		      add_opcode(splev, SPO_CMP, NULL);

		      set_opvar_int(tmppush2, splev->n_opcodes+1);

		      if_list[n_if_list++] = tmppush2;

		      add_opcode(splev, SPO_PUSH, tmppush2);

		      add_opcode(splev, reverse_jmp_opcode( (yyvsp[-1].i) ), NULL);

		  }
#line 3260 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 155: /* chancestatement: comparestmt ':' $@9 levstatement  */
#line 1136 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;

			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush,
                                        splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?!  No start address?");
		  }
#line 3274 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 156: /* $@10: %empty  */
#line 1148 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *tmppush2 = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("IF: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }

		      add_opcode(splev, SPO_CMP, NULL);

		      set_opvar_int(tmppush2, splev->n_opcodes+1);

		      if_list[n_if_list++] = tmppush2;

		      add_opcode(splev, SPO_PUSH, tmppush2);

		      add_opcode(splev, reverse_jmp_opcode( (yyvsp[0].i) ), NULL);

		  }
#line 3298 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 157: /* ifstatement: IF_ID comparestmt $@10 if_ending  */
#line 1168 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		     /* do nothing */
		  }
#line 3306 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 158: /* if_ending: stmt_block  */
#line 1174 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;

			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush,
                                        splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?!  No start address?");
		  }
#line 3320 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 159: /* $@11: %empty  */
#line 1184 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush = New(struct opvar);
			  struct opvar *tmppush2;

			  set_opvar_int(tmppush, splev->n_opcodes+1);
			  add_opcode(splev, SPO_PUSH, tmppush);

			  add_opcode(splev, SPO_JMP, NULL);

			  tmppush2 = (struct opvar *) if_list[--n_if_list];

			  set_opvar_int(tmppush2,
                                      splev->n_opcodes - tmppush2->vardata.l);
			  if_list[n_if_list++] = tmppush;
		      } else lc_error("IF: Huh?!  No else-part address?");
		  }
#line 3342 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 160: /* if_ending: stmt_block $@11 ELSE_ID stmt_block  */
#line 1202 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?! No end address?");
		  }
#line 3354 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 161: /* message: MESSAGE_ID ':' string_expr  */
#line 1212 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MESSAGE));
		  }
#line 3362 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 162: /* random_corridors: RAND_CORRIDOR_ID  */
#line 1218 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiiiio",
			      VA_PASS7(-1,  0, -1, -1, -1, -1, SPO_CORRIDOR));
		  }
#line 3371 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 163: /* random_corridors: RAND_CORRIDOR_ID ':' all_integers  */
#line 1223 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiiiio",
			      VA_PASS7(-1, (yyvsp[0].i), -1, -1, -1, -1, SPO_CORRIDOR));
		  }
#line 3380 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 164: /* random_corridors: RAND_CORRIDOR_ID ':' RANDOM_TYPE  */
#line 1228 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiiiio",
			      VA_PASS7(-1, -1, -1, -1, -1, -1, SPO_CORRIDOR));
		  }
#line 3389 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 165: /* corridor: CORRIDOR_ID ':' corr_spec ',' corr_spec  */
#line 1235 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiiiio",
				 VA_PASS7((yyvsp[-2].corpos).room, (yyvsp[-2].corpos).door, (yyvsp[-2].corpos).wall,
					  (yyvsp[0].corpos).room, (yyvsp[0].corpos).door, (yyvsp[0].corpos).wall,
					  SPO_CORRIDOR));
		  }
#line 3400 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 166: /* corridor: CORRIDOR_ID ':' corr_spec ',' all_integers  */
#line 1242 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiiiio",
				 VA_PASS7((yyvsp[-2].corpos).room, (yyvsp[-2].corpos).door, (yyvsp[-2].corpos).wall,
					  -1, -1, (long)(yyvsp[0].i),
					  SPO_CORRIDOR));
		  }
#line 3411 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 167: /* corr_spec: '(' INTEGER ',' DIRECTION ',' door_pos ')'  */
#line 1251 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.corpos).room = (yyvsp[-5].i);
			(yyval.corpos).wall = (yyvsp[-3].i);
			(yyval.corpos).door = (yyvsp[-1].i);
		  }
#line 3421 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 168: /* room_begin: room_type opt_percent ',' light_state  */
#line 1259 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (((yyvsp[-2].i) < 100) && ((yyvsp[-3].i) == OROOM))
			  lc_error("Only typed rooms can have a chance.");
		      else {
			  add_opvars(splev, "iii",
				     VA_PASS3((long)(yyvsp[-3].i), (long)(yyvsp[-2].i), (long)(yyvsp[0].i)));
		      }
                  }
#line 3434 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 169: /* $@12: %empty  */
#line 1270 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long rflags = (yyvsp[0].i);

		      if (rflags == -1) rflags = (1 << 0);
		      add_opvars(splev, "iiiiiiio",
				 VA_PASS8(rflags, ERR, ERR,
					  (yyvsp[-3].crd).x, (yyvsp[-3].crd).y, (yyvsp[-1].sze).width, (yyvsp[-1].sze).height,
					  SPO_SUBROOM));
		      break_stmt_start();
		  }
#line 3449 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 170: /* subroom_def: SUBROOM_ID ':' room_begin ',' subroom_pos ',' room_size optroomregionflags $@12 stmt_block  */
#line 1281 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      break_stmt_end(splev);
		      add_opcode(splev, SPO_ENDROOM, NULL);
		  }
#line 3458 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 171: /* $@13: %empty  */
#line 1288 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long rflags = (yyvsp[-2].i);

		      if (rflags == -1) rflags = (1 << 0);
		      add_opvars(splev, "iiiiiiio",
				 VA_PASS8(rflags,
					  (yyvsp[-3].crd).x, (yyvsp[-3].crd).y, (yyvsp[-5].crd).x, (yyvsp[-5].crd).y,
					  (yyvsp[-1].sze).width, (yyvsp[-1].sze).height, SPO_ROOM));
		      break_stmt_start();
		  }
#line 3473 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 172: /* room_def: ROOM_ID ':' room_begin ',' room_pos ',' room_align ',' room_size optroomregionflags $@13 stmt_block  */
#line 1299 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      break_stmt_end(splev);
		      add_opcode(splev, SPO_ENDROOM, NULL);
		  }
#line 3482 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 173: /* roomfill: %empty  */
#line 1306 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = 1;
		  }
#line 3490 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 174: /* roomfill: ',' BOOLEAN  */
#line 1310 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = (yyvsp[0].i);
		  }
#line 3498 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 175: /* room_pos: '(' INTEGER ',' INTEGER ')'  */
#line 1316 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			if ( (yyvsp[-3].i) < 1 || (yyvsp[-3].i) > 5 ||
			    (yyvsp[-1].i) < 1 || (yyvsp[-1].i) > 5 ) {
			    lc_error("Room positions should be between 1-5: (%li,%li)!", (yyvsp[-3].i), (yyvsp[-1].i));
			} else {
			    (yyval.crd).x = (yyvsp[-3].i);
			    (yyval.crd).y = (yyvsp[-1].i);
			}
		  }
#line 3512 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 176: /* room_pos: RANDOM_TYPE  */
#line 1326 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.crd).x = (yyval.crd).y = ERR;
		  }
#line 3520 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 177: /* subroom_pos: '(' INTEGER ',' INTEGER ')'  */
#line 1332 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			if ( (yyvsp[-3].i) < 0 || (yyvsp[-1].i) < 0) {
			    lc_error("Invalid subroom position (%li,%li)!", (yyvsp[-3].i), (yyvsp[-1].i));
			} else {
			    (yyval.crd).x = (yyvsp[-3].i);
			    (yyval.crd).y = (yyvsp[-1].i);
			}
		  }
#line 3533 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 178: /* subroom_pos: RANDOM_TYPE  */
#line 1341 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.crd).x = (yyval.crd).y = ERR;
		  }
#line 3541 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 179: /* room_align: '(' h_justif ',' v_justif ')'  */
#line 1347 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.crd).x = (yyvsp[-3].i);
		      (yyval.crd).y = (yyvsp[-1].i);
		  }
#line 3550 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 180: /* room_align: RANDOM_TYPE  */
#line 1352 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.crd).x = (yyval.crd).y = ERR;
		  }
#line 3558 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 181: /* room_size: '(' INTEGER ',' INTEGER ')'  */
#line 1358 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.sze).width = (yyvsp[-3].i);
			(yyval.sze).height = (yyvsp[-1].i);
		  }
#line 3567 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 182: /* room_size: RANDOM_TYPE  */
#line 1363 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.sze).height = (yyval.sze).width = ERR;
		  }
#line 3575 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 183: /* door_detail: ROOMDOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos  */
#line 1369 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			/* ERR means random here */
			if ((yyvsp[-2].i) == ERR && (yyvsp[0].i) != ERR) {
			    lc_error("If the door wall is random, so must be its pos!");
			} else {
			    add_opvars(splev, "iiiio",
				       VA_PASS5((long)(yyvsp[0].i), (long)(yyvsp[-4].i), (long)(yyvsp[-6].i),
						(long)(yyvsp[-2].i), SPO_ROOM_DOOR));
			}
		  }
#line 3590 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 184: /* door_detail: DOOR_ID ':' door_state ',' ter_selection  */
#line 1380 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2((long)(yyvsp[-2].i), SPO_DOOR));
		  }
#line 3598 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 189: /* dir_list: DIRECTION  */
#line 1394 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[0].i);
		  }
#line 3606 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 190: /* dir_list: DIRECTION '|' dir_list  */
#line 1398 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = ((yyvsp[-2].i) | (yyvsp[0].i));
		  }
#line 3614 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 193: /* map_definition: NOMAP_ID  */
#line 1408 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ciisiio",
				 VA_PASS7(0, 0, 1, (char *) 0, 0, 0, SPO_MAP));
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
#line 3625 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 194: /* map_definition: GEOMETRY_ID ':' h_justif ',' v_justif roomfill MAP_ID  */
#line 1415 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "cii",
				 VA_PASS3(SP_COORD_PACK(((yyvsp[-4].i)), ((yyvsp[-2].i))),
					  1, (int) (yyvsp[-1].i)));
		      scan_map((yyvsp[0].map), splev);
		      Free((yyvsp[0].map));
		  }
#line 3637 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 195: /* map_definition: GEOMETRY_ID ':' coord_or_var roomfill MAP_ID  */
#line 1423 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(2, (int) (yyvsp[-1].i)));
		      scan_map((yyvsp[0].map), splev);
		      Free((yyvsp[0].map));
		  }
#line 3647 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 200: /* monster_detail: MONSTER_ID ':' monster_desc  */
#line 1439 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(0, SPO_MONSTER));
		  }
#line 3655 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 201: /* $@14: %empty  */
#line 1443 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(1, SPO_MONSTER));
		      in_container_obj++;
		      break_stmt_start();
		  }
#line 3665 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 202: /* monster_detail: MONSTER_ID ':' monster_desc $@14 stmt_block  */
#line 1449 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                 {
		     break_stmt_end(splev);
		     in_container_obj--;
		     add_opvars(splev, "o", VA_PASS1(SPO_END_MONINVENT));
		 }
#line 3675 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 203: /* monster_desc: monster_or_var ',' coord_or_var monster_infos  */
#line 1457 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 3683 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 204: /* monster_infos: %empty  */
#line 1463 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *stopit = New(struct opvar);

		      set_opvar_int(stopit, SP_M_V_END);
		      add_opcode(splev, SPO_PUSH, stopit);
		      (yyval.i) = 0x0000;
		  }
#line 3695 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 205: /* monster_infos: monster_infos ',' monster_info  */
#line 1471 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (( (yyvsp[-2].i) & (yyvsp[0].i) ))
			  lc_error("MONSTER extra info defined twice.");
		      (yyval.i) = ( (yyvsp[-2].i) | (yyvsp[0].i) );
		  }
#line 3705 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 206: /* monster_info: string_expr  */
#line 1479 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_M_V_NAME));
		      (yyval.i) = 0x0001;
		  }
#line 3714 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 207: /* monster_info: MON_ATTITUDE  */
#line 1484 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
				 VA_PASS2((int) (yyvsp[0].i), SP_M_V_PEACEFUL));
		      (yyval.i) = 0x0002;
		  }
#line 3724 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 208: /* monster_info: MON_ALERTNESS  */
#line 1490 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
				 VA_PASS2((int) (yyvsp[0].i), SP_M_V_ASLEEP));
		      (yyval.i) = 0x0004;
		  }
#line 3734 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 209: /* monster_info: alignment_prfx  */
#line 1496 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
				 VA_PASS2((int) (yyvsp[0].i), SP_M_V_ALIGN));
		      (yyval.i) = 0x0008;
		  }
#line 3744 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 210: /* monster_info: MON_APPEARANCE string_expr  */
#line 1502 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
				 VA_PASS2((int) (yyvsp[-1].i), SP_M_V_APPEAR));
		      (yyval.i) = 0x0010;
		  }
#line 3754 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 211: /* monster_info: FEMALE_ID  */
#line 1508 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_FEMALE));
		      (yyval.i) = 0x0020;
		  }
#line 3763 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 212: /* monster_info: INVIS_ID  */
#line 1513 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_INVIS));
		      (yyval.i) = 0x0040;
		  }
#line 3772 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 213: /* monster_info: CANCELLED_ID  */
#line 1518 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_CANCELLED));
		      (yyval.i) = 0x0080;
		  }
#line 3781 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 214: /* monster_info: REVIVED_ID  */
#line 1523 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_REVIVED));
		      (yyval.i) = 0x0100;
		  }
#line 3790 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 215: /* monster_info: AVENGE_ID  */
#line 1528 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_AVENGE));
		      (yyval.i) = 0x0200;
		  }
#line 3799 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 216: /* monster_info: FLEEING_ID ':' integer_or_var  */
#line 1533 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_M_V_FLEEING));
		      (yyval.i) = 0x0400;
		  }
#line 3808 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 217: /* monster_info: BLINDED_ID ':' integer_or_var  */
#line 1538 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_M_V_BLINDED));
		      (yyval.i) = 0x0800;
		  }
#line 3817 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 218: /* monster_info: PARALYZED_ID ':' integer_or_var  */
#line 1543 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_M_V_PARALYZED));
		      (yyval.i) = 0x1000;
		  }
#line 3826 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 219: /* monster_info: STUNNED_ID  */
#line 1548 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_STUNNED));
		      (yyval.i) = 0x2000;
		  }
#line 3835 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 220: /* monster_info: CONFUSED_ID  */
#line 1553 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_M_V_CONFUSED));
		      (yyval.i) = 0x4000;
		  }
#line 3844 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 221: /* monster_info: SEENTRAPS_ID ':' seen_trap_mask  */
#line 1558 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
				 VA_PASS2((int) (yyvsp[0].i), SP_M_V_SEENTRAPS));
		      (yyval.i) = 0x8000;
		  }
#line 3854 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 222: /* seen_trap_mask: STRING  */
#line 1566 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int token = get_trap_type((yyvsp[0].map));

		      if (token == ERR || token == 0)
			  lc_error("Unknown trap type '%s'!", (yyvsp[0].map));
                      Free((yyvsp[0].map));
		      (yyval.i) = (1L << (token - 1));
		  }
#line 3867 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 223: /* seen_trap_mask: ALL_ID  */
#line 1575 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (long) ~0;
		  }
#line 3875 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 224: /* seen_trap_mask: STRING '|' seen_trap_mask  */
#line 1579 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int token = get_trap_type((yyvsp[-2].map));
		      if (token == ERR || token == 0)
			  lc_error("Unknown trap type '%s'!", (yyvsp[-2].map));

		      if ((1L << (token - 1)) & (yyvsp[0].i))
			  lc_error("Monster seen_traps, trap '%s' listed twice.", (yyvsp[-2].map));
                      Free((yyvsp[-2].map));
		      (yyval.i) = ((1L << (token - 1)) | (yyvsp[0].i));
		  }
#line 3890 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 225: /* object_detail: OBJECT_ID ':' object_desc  */
#line 1592 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int cnt = 0;

		      if (in_container_obj)
                          cnt |= SP_OBJ_CONTENT;
		      add_opvars(splev, "io", VA_PASS2(cnt, SPO_OBJECT));
		  }
#line 3902 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 226: /* $@15: %empty  */
#line 1600 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      int cnt = SP_OBJ_CONTAINER;

		      if (in_container_obj)
                          cnt |= SP_OBJ_CONTENT;
		      add_opvars(splev, "io", VA_PASS2(cnt, SPO_OBJECT));
		      in_container_obj++;
		      break_stmt_start();
		  }
#line 3916 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 227: /* object_detail: COBJECT_ID ':' object_desc $@15 stmt_block  */
#line 1610 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                 {
		     break_stmt_end(splev);
		     in_container_obj--;
		     add_opcode(splev, SPO_POP_CONTAINER, NULL);
		 }
#line 3926 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 228: /* object_desc: object_or_var object_infos  */
#line 1618 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (( (yyvsp[0].i) & 0x4000) && in_container_obj)
                          lc_error("Object cannot have a coord when contained.");
		      else if (!( (yyvsp[0].i) & 0x4000) && !in_container_obj)
                          lc_error("Object needs a coord when not contained.");
		  }
#line 3937 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 229: /* object_infos: %empty  */
#line 1627 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_O_V_END);
		      add_opcode(splev, SPO_PUSH, stopit);
		      (yyval.i) = 0x00;
		  }
#line 3948 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 230: /* object_infos: object_infos ',' object_info  */
#line 1634 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (( (yyvsp[-2].i) & (yyvsp[0].i) ))
			  lc_error("OBJECT extra info '%s' defined twice.", curr_token);
		      (yyval.i) = ( (yyvsp[-2].i) | (yyvsp[0].i) );
		  }
#line 3958 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 231: /* object_info: CURSE_TYPE  */
#line 1642 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
				 VA_PASS2((int) (yyvsp[0].i), SP_O_V_CURSE));
		      (yyval.i) = 0x0001;
		  }
#line 3968 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 232: /* object_info: MONTYPE_ID ':' monster_or_var  */
#line 1648 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_CORPSENM));
		      (yyval.i) = 0x0002;
		  }
#line 3977 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 233: /* object_info: all_ints_push  */
#line 1653 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_SPE));
		      (yyval.i) = 0x0004;
		  }
#line 3986 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 234: /* object_info: NAME_ID ':' string_expr  */
#line 1658 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_NAME));
		      (yyval.i) = 0x0008;
		  }
#line 3995 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 235: /* object_info: QUANTITY_ID ':' integer_or_var  */
#line 1663 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_QUAN));
		      (yyval.i) = 0x0010;
		  }
#line 4004 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 236: /* object_info: BURIED_ID  */
#line 1668 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_O_V_BURIED));
		      (yyval.i) = 0x0020;
		  }
#line 4013 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 237: /* object_info: LIGHT_STATE  */
#line 1673 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2((int) (yyvsp[0].i), SP_O_V_LIT));
		      (yyval.i) = 0x0040;
		  }
#line 4022 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 238: /* object_info: ERODED_ID ':' integer_or_var  */
#line 1678 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_ERODED));
		      (yyval.i) = 0x0080;
		  }
#line 4031 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 239: /* object_info: ERODEPROOF_ID  */
#line 1683 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(-1, SP_O_V_ERODED));
		      (yyval.i) = 0x0080;
		  }
#line 4040 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 240: /* object_info: DOOR_STATE  */
#line 1688 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if ((yyvsp[0].i) == D_LOCKED) {
			  add_opvars(splev, "ii", VA_PASS2(1, SP_O_V_LOCKED));
			  (yyval.i) = 0x0100;
		      } else if ((yyvsp[0].i) == D_BROKEN) {
			  add_opvars(splev, "ii", VA_PASS2(1, SP_O_V_BROKEN));
			  (yyval.i) = 0x0200;
		      } else
			  lc_error("DOOR state can only be locked or broken.");
		  }
#line 4055 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 241: /* object_info: TRAPPED_STATE  */
#line 1699 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii",
                                 VA_PASS2((int) (yyvsp[0].i), SP_O_V_TRAPPED));
		      (yyval.i) = 0x0400;
		  }
#line 4065 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 242: /* object_info: RECHARGED_ID ':' integer_or_var  */
#line 1705 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_RECHARGED));
		      (yyval.i) = 0x0800;
		  }
#line 4074 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 243: /* object_info: INVIS_ID  */
#line 1710 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_O_V_INVIS));
		      (yyval.i) = 0x1000;
		  }
#line 4083 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 244: /* object_info: GREASED_ID  */
#line 1715 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "ii", VA_PASS2(1, SP_O_V_GREASED));
		      (yyval.i) = 0x2000;
		  }
#line 4092 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 245: /* object_info: coord_or_var  */
#line 1720 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1(SP_O_V_COORD));
		      (yyval.i) = 0x4000;
		  }
#line 4101 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 246: /* trap_detail: TRAP_ID ':' trap_name ',' coord_or_var  */
#line 1727 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2((int) (yyvsp[-2].i), SPO_TRAP));
		  }
#line 4109 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 247: /* drawbridge_detail: DRAWBRIDGE_ID ':' coord_or_var ',' DIRECTION ',' door_state  */
#line 1733 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                   {
		       long dir, state = 0;

		       /* convert dir from a DIRECTION to a DB_DIR */
		       dir = (yyvsp[-2].i);
		       switch (dir) {
		       case W_NORTH: dir = DB_NORTH; break;
		       case W_SOUTH: dir = DB_SOUTH; break;
		       case W_EAST:  dir = DB_EAST;  break;
		       case W_WEST:  dir = DB_WEST;  break;
		       default:
			   lc_error("Invalid drawbridge direction.");
			   break;
		       }

		       if ( (yyvsp[0].i) == D_ISOPEN )
			   state = 1;
		       else if ( (yyvsp[0].i) == D_CLOSED )
			   state = 0;
		       else if ( (yyvsp[0].i) == -1 )
			   state = -1;
		       else
			   lc_error("A drawbridge can only be open, closed or random!");
		       add_opvars(splev, "iio",
				  VA_PASS3(state, dir, SPO_DRAWBRIDGE));
		   }
#line 4140 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 248: /* mazewalk_detail: MAZEWALK_ID ':' coord_or_var ',' DIRECTION  */
#line 1762 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiio",
				 VA_PASS4((int) (yyvsp[0].i), 1, 0, SPO_MAZEWALK));
		  }
#line 4149 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 249: /* mazewalk_detail: MAZEWALK_ID ':' coord_or_var ',' DIRECTION ',' BOOLEAN opt_fillchar  */
#line 1767 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiio",
				 VA_PASS4((int) (yyvsp[-3].i), (int) (yyvsp[-1].i),
					  (int) (yyvsp[0].i), SPO_MAZEWALK));
		  }
#line 4159 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 250: /* wallify_detail: WALLIFY_ID  */
#line 1775 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "rio",
				 VA_PASS3(SP_REGION_PACK(-1,-1,-1,-1),
					  0, SPO_WALLIFY));
		  }
#line 4169 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 251: /* wallify_detail: WALLIFY_ID ':' ter_selection  */
#line 1781 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(1, SPO_WALLIFY));
		  }
#line 4177 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 252: /* ladder_detail: LADDER_ID ':' coord_or_var ',' UP_OR_DOWN  */
#line 1787 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
				 VA_PASS2((int) (yyvsp[0].i), SPO_LADDER));
		  }
#line 4186 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 253: /* stair_detail: STAIR_ID ':' coord_or_var ',' UP_OR_DOWN  */
#line 1794 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
				 VA_PASS2((int) (yyvsp[0].i), SPO_STAIR));
		  }
#line 4195 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 254: /* stair_region: STAIR_ID ':' lev_region ',' lev_region ',' UP_OR_DOWN  */
#line 1801 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiii iiiii iiso",
				 VA_PASS14((yyvsp[-4].lregn).x1, (yyvsp[-4].lregn).y1, (yyvsp[-4].lregn).x2, (yyvsp[-4].lregn).y2, (yyvsp[-4].lregn).area,
					   (yyvsp[-2].lregn).x1, (yyvsp[-2].lregn).y1, (yyvsp[-2].lregn).x2, (yyvsp[-2].lregn).y2, (yyvsp[-2].lregn).area,
				     (long) (((yyvsp[0].i)) ? LR_UPSTAIR : LR_DOWNSTAIR),
					   0, (char *) 0, SPO_LEVREGION));
		  }
#line 4207 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 255: /* portal_region: PORTAL_ID ':' lev_region ',' lev_region ',' STRING  */
#line 1811 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiii iiiii iiso",
				 VA_PASS14((yyvsp[-4].lregn).x1, (yyvsp[-4].lregn).y1, (yyvsp[-4].lregn).x2, (yyvsp[-4].lregn).y2, (yyvsp[-4].lregn).area,
					   (yyvsp[-2].lregn).x1, (yyvsp[-2].lregn).y1, (yyvsp[-2].lregn).x2, (yyvsp[-2].lregn).y2, (yyvsp[-2].lregn).area,
					   LR_PORTAL, 0, (yyvsp[0].map), SPO_LEVREGION));
		      Free((yyvsp[0].map));
		  }
#line 4219 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 256: /* teleprt_region: TELEPRT_ID ':' lev_region ',' lev_region teleprt_detail  */
#line 1821 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long rtyp = 0;
		      switch((yyvsp[0].i)) {
		      case -1: rtyp = LR_TELE; break;
		      case  0: rtyp = LR_DOWNTELE; break;
		      case  1: rtyp = LR_UPTELE; break;
		      }
		      add_opvars(splev, "iiiii iiiii iiso",
				 VA_PASS14((yyvsp[-3].lregn).x1, (yyvsp[-3].lregn).y1, (yyvsp[-3].lregn).x2, (yyvsp[-3].lregn).y2, (yyvsp[-3].lregn).area,
					   (yyvsp[-1].lregn).x1, (yyvsp[-1].lregn).y1, (yyvsp[-1].lregn).x2, (yyvsp[-1].lregn).y2, (yyvsp[-1].lregn).area,
					   rtyp, 0, (char *)0, SPO_LEVREGION));
		  }
#line 4236 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 257: /* branch_region: BRANCH_ID ':' lev_region ',' lev_region  */
#line 1836 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiii iiiii iiso",
				 VA_PASS14((yyvsp[-2].lregn).x1, (yyvsp[-2].lregn).y1, (yyvsp[-2].lregn).x2, (yyvsp[-2].lregn).y2, (yyvsp[-2].lregn).area,
					   (yyvsp[0].lregn).x1, (yyvsp[0].lregn).y1, (yyvsp[0].lregn).x2, (yyvsp[0].lregn).y2, (yyvsp[0].lregn).area,
					   (long) LR_BRANCH, 0,
					   (char *) 0, SPO_LEVREGION));
		  }
#line 4248 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 258: /* teleprt_detail: %empty  */
#line 1846 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = -1;
		  }
#line 4256 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 259: /* teleprt_detail: ',' UP_OR_DOWN  */
#line 1850 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = (yyvsp[0].i);
		  }
#line 4264 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 260: /* fountain_detail: FOUNTAIN_ID ':' ter_selection  */
#line 1856 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_FOUNTAIN));
		  }
#line 4272 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 261: /* sink_detail: SINK_ID ':' ter_selection  */
#line 1862 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SINK));
		  }
#line 4280 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 262: /* pool_detail: POOL_ID ':' ter_selection  */
#line 1868 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_POOL));
		  }
#line 4288 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 263: /* terrain_type: CHAR  */
#line 1874 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.terr).lit = -2;
		      (yyval.terr).ter = what_map_char((char) (yyvsp[0].i));
		  }
#line 4297 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 264: /* terrain_type: '(' CHAR ',' light_state ')'  */
#line 1879 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.terr).lit = (yyvsp[-1].i);
		      (yyval.terr).ter = what_map_char((char) (yyvsp[-3].i));
		  }
#line 4306 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 265: /* replace_terrain_detail: REPLACE_TERRAIN_ID ':' region_or_var ',' mapchar_or_var ',' mapchar_or_var ',' SPERCENT  */
#line 1886 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
				 VA_PASS2((yyvsp[0].i), SPO_REPLACETERRAIN));
		  }
#line 4315 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 266: /* terrain_detail: TERRAIN_ID ':' ter_selection ',' mapchar_or_var  */
#line 1893 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                 {
		     add_opvars(splev, "o", VA_PASS1(SPO_TERRAIN));
		 }
#line 4323 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 267: /* diggable_detail: NON_DIGGABLE_ID ':' region_or_var  */
#line 1899 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_NON_DIGGABLE));
		  }
#line 4331 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 268: /* passwall_detail: NON_PASSWALL_ID ':' region_or_var  */
#line 1905 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_NON_PASSWALL));
		  }
#line 4339 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 269: /* @16: %empty  */
#line 1911 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long irr;
		      long rt = (yyvsp[-1].i);
		      long rflags = (yyvsp[0].i);

		      if (rflags == -1) rflags = (1 << 0);
		      if (!(rflags & 1)) rt += MAXRTYPE+1;
		      irr = ((rflags & 2) != 0);
		      add_opvars(splev, "iiio",
				 VA_PASS4((long)(yyvsp[-3].i), rt, rflags, SPO_REGION));
		      (yyval.i) = (irr || (rflags & 1) || rt != OROOM);
		      break_stmt_start();
		  }
#line 4357 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 270: /* region_detail: REGION_ID ':' region_or_var ',' light_state ',' room_type optroomregionflags @16 region_detail_end  */
#line 1925 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      break_stmt_end(splev);
		      if ( (yyvsp[-1].i) ) {
			  add_opcode(splev, SPO_ENDROOM, NULL);
		      } else if ( (yyvsp[0].i) )
			  lc_error("Cannot use lev statements in non-permanent REGION");
		  }
#line 4369 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 271: /* region_detail_end: %empty  */
#line 1935 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = 0;
		  }
#line 4377 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 272: /* region_detail_end: stmt_block  */
#line 1939 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[0].i);
		  }
#line 4385 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 273: /* altar_detail: ALTAR_ID ':' coord_or_var ',' alignment ',' altar_type  */
#line 1945 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iio",
				 VA_PASS3((long)(yyvsp[0].i), (long)(yyvsp[-2].i), SPO_ALTAR));
		  }
#line 4394 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 274: /* grave_detail: GRAVE_ID ':' coord_or_var ',' string_expr  */
#line 1952 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(2, SPO_GRAVE));
		  }
#line 4402 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 275: /* grave_detail: GRAVE_ID ':' coord_or_var ',' RANDOM_TYPE  */
#line 1956 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "sio",
				 VA_PASS3((char *)0, 1, SPO_GRAVE));
		  }
#line 4411 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 276: /* grave_detail: GRAVE_ID ':' coord_or_var  */
#line 1961 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "sio",
				 VA_PASS3((char *)0, 0, SPO_GRAVE));
		  }
#line 4420 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 277: /* gold_detail: GOLD_ID ':' math_expr_var ',' coord_or_var  */
#line 1968 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_GOLD));
		  }
#line 4428 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 278: /* engraving_detail: ENGRAVING_ID ':' coord_or_var ',' engraving_type ',' string_expr  */
#line 1974 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
				 VA_PASS2((long)(yyvsp[-2].i), SPO_ENGRAVING));
		  }
#line 4437 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 279: /* mineralize: MINERALIZE_ID ':' integer_or_var ',' integer_or_var ',' integer_or_var ',' integer_or_var  */
#line 1981 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MINERALIZE));
		  }
#line 4445 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 280: /* mineralize: MINERALIZE_ID  */
#line 1985 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iiiio",
				 VA_PASS5(-1L, -1L, -1L, -1L, SPO_MINERALIZE));
		  }
#line 4454 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 281: /* trap_name: STRING  */
#line 1992 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			int token = get_trap_type((yyvsp[0].map));
			if (token == ERR)
			    lc_error("Unknown trap type '%s'!", (yyvsp[0].map));
			(yyval.i) = token;
			Free((yyvsp[0].map));
		  }
#line 4466 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 283: /* room_type: STRING  */
#line 2003 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			int token = get_room_type((yyvsp[0].map));
			if (token == ERR) {
			    lc_warning("Unknown room type \"%s\"!  Making ordinary room...", (yyvsp[0].map));
				(yyval.i) = OROOM;
			} else
				(yyval.i) = token;
			Free((yyvsp[0].map));
		  }
#line 4480 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 285: /* optroomregionflags: %empty  */
#line 2016 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = -1;
		  }
#line 4488 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 286: /* optroomregionflags: ',' roomregionflags  */
#line 2020 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = (yyvsp[0].i);
		  }
#line 4496 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 287: /* roomregionflags: roomregionflag  */
#line 2026 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = (yyvsp[0].i);
		  }
#line 4504 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 288: /* roomregionflags: roomregionflag ',' roomregionflags  */
#line 2030 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = (yyvsp[-2].i) | (yyvsp[0].i);
		  }
#line 4512 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 289: /* roomregionflag: FILLING  */
#line 2037 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = ((yyvsp[0].i) << 0);
		  }
#line 4520 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 290: /* roomregionflag: IRREGULAR  */
#line 2041 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = ((yyvsp[0].i) << 1);
		  }
#line 4528 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 291: /* roomregionflag: JOINED  */
#line 2045 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = ((yyvsp[0].i) << 2);
		  }
#line 4536 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 298: /* alignment: RANDOM_TYPE  */
#line 2061 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = - MAX_REGISTERS - 1;
		  }
#line 4544 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 301: /* alignment_prfx: A_REGISTER ':' RANDOM_TYPE  */
#line 2069 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.i) = - MAX_REGISTERS - 1;
		  }
#line 4552 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 304: /* a_register: A_REGISTER '[' INTEGER ']'  */
#line 2079 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			if ( (yyvsp[-1].i) >= 3 )
				lc_error("Register Index overflow!");
			else
				(yyval.i) = - (yyvsp[-1].i) - 1;
		  }
#line 4563 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 305: /* string_or_var: STRING  */
#line 2088 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "s", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4572 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 306: /* string_or_var: VARSTRING_STRING  */
#line 2093 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_STRING);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4583 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 307: /* string_or_var: VARSTRING_STRING_ARRAY '[' math_expr_var ']'  */
#line 2100 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
                                        SPOVAR_STRING | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		  }
#line 4595 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 308: /* integer_or_var: math_expr_var  */
#line 2111 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 4603 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 309: /* coord_or_var: encodecoord  */
#line 2117 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "c", VA_PASS1((yyvsp[0].i)));
		  }
#line 4611 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 310: /* coord_or_var: rndcoord_ID '(' ter_selection ')'  */
#line 2121 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_RNDCOORD));
		  }
#line 4619 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 311: /* coord_or_var: VARSTRING_COORD  */
#line 2125 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_COORD);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4630 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 312: /* coord_or_var: VARSTRING_COORD_ARRAY '[' math_expr_var ']'  */
#line 2132 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
                                        SPOVAR_COORD | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		  }
#line 4642 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 313: /* encodecoord: '(' INTEGER ',' INTEGER ')'  */
#line 2142 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if ((yyvsp[-3].i) < 0 || (yyvsp[-1].i) < 0 || (yyvsp[-3].i) >= COLNO || (yyvsp[-1].i) >= ROWNO)
                          lc_error("Coordinates (%li,%li) out of map range!",
                                   (yyvsp[-3].i), (yyvsp[-1].i));
		      (yyval.i) = SP_COORD_PACK((yyvsp[-3].i), (yyvsp[-1].i));
		  }
#line 4653 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 314: /* encodecoord: RANDOM_TYPE  */
#line 2149 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = SP_COORD_PACK_RANDOM(0);
		  }
#line 4661 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 315: /* encodecoord: RANDOM_TYPE_BRACKET humidity_flags ']'  */
#line 2153 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = SP_COORD_PACK_RANDOM((yyvsp[-1].i));
		  }
#line 4669 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 316: /* humidity_flags: HUMIDITY_TYPE  */
#line 2159 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = (yyvsp[0].i);
		  }
#line 4677 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 317: /* humidity_flags: HUMIDITY_TYPE ',' humidity_flags  */
#line 2163 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (((yyvsp[-2].i) & (yyvsp[0].i)))
			  lc_warning("Humidity flag used twice.");
		      (yyval.i) = ((yyvsp[-2].i) | (yyvsp[0].i));
		  }
#line 4687 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 318: /* region_or_var: encoderegion  */
#line 2171 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 4695 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 319: /* region_or_var: VARSTRING_REGION  */
#line 2175 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_REGION);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4706 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 320: /* region_or_var: VARSTRING_REGION_ARRAY '[' math_expr_var ']'  */
#line 2182 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
                                        SPOVAR_REGION | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		  }
#line 4718 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 321: /* encoderegion: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'  */
#line 2192 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long r = SP_REGION_PACK((yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));

		      if ((yyvsp[-7].i) > (yyvsp[-3].i) || (yyvsp[-5].i) > (yyvsp[-1].i))
			  lc_error("Region start > end: (%ld,%ld,%ld,%ld)!",
                                   (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));

		      add_opvars(splev, "r", VA_PASS1(r));
		      (yyval.i) = r;
		  }
#line 4733 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 322: /* mapchar_or_var: mapchar  */
#line 2205 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "m", VA_PASS1((yyvsp[0].i)));
		  }
#line 4741 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 323: /* mapchar_or_var: VARSTRING_MAPCHAR  */
#line 2209 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_MAPCHAR);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4752 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 324: /* mapchar_or_var: VARSTRING_MAPCHAR_ARRAY '[' math_expr_var ']'  */
#line 2216 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
                                        SPOVAR_MAPCHAR | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		  }
#line 4764 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 325: /* mapchar: CHAR  */
#line 2226 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (what_map_char((char) (yyvsp[0].i)) != INVALID_TYPE)
			  (yyval.i) = SP_MAPCHAR_PACK(what_map_char((char) (yyvsp[0].i)), -2);
		      else {
			  lc_error("Unknown map char type '%c'!", (yyvsp[0].i));
			  (yyval.i) = SP_MAPCHAR_PACK(STONE, -2);
		      }
		  }
#line 4777 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 326: /* mapchar: '(' CHAR ',' light_state ')'  */
#line 2235 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (what_map_char((char) (yyvsp[-3].i)) != INVALID_TYPE)
			  (yyval.i) = SP_MAPCHAR_PACK(what_map_char((char) (yyvsp[-3].i)), (yyvsp[-1].i));
		      else {
			  lc_error("Unknown map char type '%c'!", (yyvsp[-3].i));
			  (yyval.i) = SP_MAPCHAR_PACK(STONE, (yyvsp[-1].i));
		      }
		  }
#line 4790 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 327: /* monster_or_var: encodemonster  */
#line 2246 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "M", VA_PASS1((yyvsp[0].i)));
		  }
#line 4798 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 328: /* monster_or_var: VARSTRING_MONST  */
#line 2250 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_MONST);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4809 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 329: /* monster_or_var: VARSTRING_MONST_ARRAY '[' math_expr_var ']'  */
#line 2257 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
                                        SPOVAR_MONST | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		  }
#line 4821 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 330: /* encodemonster: STRING  */
#line 2267 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      long m = get_monster_id((yyvsp[0].map), (char)0);
                      if (m == ERR) {
                          lc_error("Unknown monster \"%s\"!", (yyvsp[0].map));
                          (yyval.i) = -1;
                      } else
                          (yyval.i) = SP_MONST_PACK(m,
                                         def_monsyms[(int) mons[m].mlet].sym);
                      Free((yyvsp[0].map));
                  }
#line 4836 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 331: /* encodemonster: CHAR  */
#line 2278 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                        if (check_monster_char((char) (yyvsp[0].i)))
                            (yyval.i) = SP_MONST_PACK(-1, (yyvsp[0].i));
                        else {
                            lc_error("Unknown monster class '%c'!", (yyvsp[0].i));
                            (yyval.i) = -1;
                        }
                  }
#line 4849 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 332: /* encodemonster: '(' CHAR ',' STRING ')'  */
#line 2287 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      long m = get_monster_id((yyvsp[-1].map), (char) (yyvsp[-3].i));
                      if (m == ERR) {
                          lc_error("Unknown monster ('%c', \"%s\")!", (yyvsp[-3].i), (yyvsp[-1].map));
                          (yyval.i) = -1;
                      } else
                          (yyval.i) = SP_MONST_PACK(m, (yyvsp[-3].i));
                      Free((yyvsp[-1].map));
                  }
#line 4863 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 333: /* encodemonster: RANDOM_TYPE  */
#line 2297 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
                      (yyval.i) = -1;
                  }
#line 4871 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 334: /* object_or_var: encodeobj  */
#line 2303 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "O", VA_PASS1((yyvsp[0].i)));
		  }
#line 4879 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 335: /* object_or_var: VARSTRING_OBJ  */
#line 2307 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_OBJ);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 4890 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 336: /* object_or_var: VARSTRING_OBJ_ARRAY '[' math_expr_var ']'  */
#line 2314 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
                                        SPOVAR_OBJ | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		  }
#line 4902 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 337: /* encodeobj: STRING  */
#line 2324 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long m = get_object_id((yyvsp[0].map), (char)0);
		      if (m == ERR) {
			  lc_error("Unknown object \"%s\"!", (yyvsp[0].map));
			  (yyval.i) = -1;
		      } else
                          /* obj class != 0 to force generation of a specific item */
                          (yyval.i) = SP_OBJ_PACK(m, 1);
                      Free((yyvsp[0].map));
		  }
#line 4917 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 338: /* encodeobj: CHAR  */
#line 2335 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			if (check_object_char((char) (yyvsp[0].i)))
			    (yyval.i) = SP_OBJ_PACK(-1, (yyvsp[0].i));
			else {
			    lc_error("Unknown object class '%c'!", (yyvsp[0].i));
			    (yyval.i) = -1;
			}
		  }
#line 4930 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 339: /* encodeobj: '(' CHAR ',' STRING ')'  */
#line 2344 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      long m = get_object_id((yyvsp[-1].map), (char) (yyvsp[-3].i));
		      if (m == ERR) {
			  lc_error("Unknown object ('%c', \"%s\")!", (yyvsp[-3].i), (yyvsp[-1].map));
			  (yyval.i) = -1;
		      } else
			  (yyval.i) = SP_OBJ_PACK(m, (yyvsp[-3].i));
                      Free((yyvsp[-1].map));
		  }
#line 4944 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 340: /* encodeobj: RANDOM_TYPE  */
#line 2354 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      (yyval.i) = -1;
		  }
#line 4952 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 341: /* string_expr: string_or_var  */
#line 2360 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                                                { }
#line 4958 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 342: /* string_expr: string_expr '.' string_or_var  */
#line 2362 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_ADD));
		  }
#line 4966 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 343: /* math_expr_var: INTEGER  */
#line 2368 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1((yyvsp[0].i)));
		  }
#line 4974 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 344: /* math_expr_var: dice  */
#line 2372 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      is_inconstant_number = 1;
		  }
#line 4982 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 345: /* math_expr_var: '(' MINUS_INTEGER ')'  */
#line 2376 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1((yyvsp[-1].i)));
		  }
#line 4990 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 346: /* math_expr_var: VARSTRING_INT  */
#line 2380 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_INT);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		      is_inconstant_number = 1;
		  }
#line 5002 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 347: /* math_expr_var: VARSTRING_INT_ARRAY '[' math_expr_var ']'  */
#line 2388 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[-3].map),
					SPOVAR_INT | SPOVAR_ARRAY);
		      vardef_used(vardefs, (yyvsp[-3].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[-3].map)));
		      Free((yyvsp[-3].map));
		      is_inconstant_number = 1;
		  }
#line 5015 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 348: /* math_expr_var: math_expr_var '+' math_expr_var  */
#line 2397 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_ADD));
		  }
#line 5023 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 349: /* math_expr_var: math_expr_var '-' math_expr_var  */
#line 2401 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_SUB));
		  }
#line 5031 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 350: /* math_expr_var: math_expr_var '*' math_expr_var  */
#line 2405 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_MUL));
		  }
#line 5039 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 351: /* math_expr_var: math_expr_var '/' math_expr_var  */
#line 2409 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_DIV));
		  }
#line 5047 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 352: /* math_expr_var: math_expr_var '%' math_expr_var  */
#line 2413 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_MATH_MOD));
		  }
#line 5055 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 353: /* math_expr_var: '(' math_expr_var ')'  */
#line 2416 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                                                    { }
#line 5061 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 354: /* func_param_type: CFUNC_INT  */
#line 2420 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (!strcmp("int", (yyvsp[0].map)) || !strcmp("integer", (yyvsp[0].map))) {
			  (yyval.i) = (int)'i';
		      } else
			  lc_error("Unknown function parameter type '%s'", (yyvsp[0].map));
		  }
#line 5072 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 355: /* func_param_type: CFUNC_STR  */
#line 2427 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      if (!strcmp("str", (yyvsp[0].map)) || !strcmp("string", (yyvsp[0].map))) {
			  (yyval.i) = (int)'s';
		      } else
			  lc_error("Unknown function parameter type '%s'", (yyvsp[0].map));
		  }
#line 5083 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 356: /* func_param_part: any_var_or_arr ':' func_param_type  */
#line 2436 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      struct lc_funcdefs_parm *tmp = New(struct lc_funcdefs_parm);

		      if (!curr_function) {
			  lc_error("Function parameters outside function definition.");
		      } else if (!tmp) {
			  lc_error("Could not alloc function params.");
		      } else {
			  long vt = SPOVAR_NULL;

			  tmp->name = strdup((yyvsp[-2].map));
			  tmp->parmtype = (char) (yyvsp[0].i);
			  tmp->next = curr_function->params;
			  curr_function->params = tmp;
			  curr_function->n_params++;
			  switch (tmp->parmtype) {
			  case 'i':
                              vt = SPOVAR_INT;
                              break;
			  case 's':
                              vt = SPOVAR_STRING;
                              break;
			  default:
                              lc_error("Unknown func param conversion.");
                              break;
			  }
			  vardefs = add_vardef_type( vardefs, (yyvsp[-2].map), vt);
		      }
		      Free((yyvsp[-2].map));
		  }
#line 5118 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 361: /* func_call_param_part: math_expr_var  */
#line 2477 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                          {
			      (yyval.i) = (int)'i';
			  }
#line 5126 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 362: /* func_call_param_part: string_expr  */
#line 2481 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                          {
			      (yyval.i) = (int)'s';
			  }
#line 5134 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 363: /* func_call_param_list: func_call_param_part  */
#line 2488 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                          {
			      char tmpbuf[2];
			      tmpbuf[0] = (char) (yyvsp[0].i);
			      tmpbuf[1] = '\0';
			      (yyval.map) = strdup(tmpbuf);
			  }
#line 5145 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 364: /* func_call_param_list: func_call_param_list ',' func_call_param_part  */
#line 2495 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                          {
			      long len = strlen( (yyvsp[-2].map) );
			      char *tmp = (char *) alloc(len + 2);
			      sprintf(tmp, "%c%s", (char) (yyvsp[0].i), (yyvsp[-2].map) );
			      Free( (yyvsp[-2].map) );
			      (yyval.map) = tmp;
			  }
#line 5157 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 365: /* func_call_params_list: %empty  */
#line 2505 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                          {
			      (yyval.map) = strdup("");
			  }
#line 5165 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 366: /* func_call_params_list: func_call_param_list  */
#line 2509 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                          {
			      char *tmp = strdup( (yyvsp[0].map) );
			      Free( (yyvsp[0].map) );
			      (yyval.map) = tmp;
			  }
#line 5175 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 367: /* ter_selection_x: coord_or_var  */
#line 2517 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_POINT));
		  }
#line 5183 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 368: /* ter_selection_x: rect_ID region_or_var  */
#line 2521 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_RECT));
		  }
#line 5191 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 369: /* ter_selection_x: fillrect_ID region_or_var  */
#line 2525 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_FILLRECT));
		  }
#line 5199 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 370: /* ter_selection_x: line_ID coord_or_var ',' coord_or_var  */
#line 2529 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_LINE));
		  }
#line 5207 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 371: /* ter_selection_x: randline_ID coord_or_var ',' coord_or_var ',' math_expr_var  */
#line 2533 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* randline (x1,y1),(x2,y2), roughness */
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_RNDLINE));
		  }
#line 5216 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 372: /* ter_selection_x: grow_ID '(' ter_selection ')'  */
#line 2538 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(W_ANY, SPO_SEL_GROW));
		  }
#line 5224 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 373: /* ter_selection_x: grow_ID '(' dir_list ',' ter_selection ')'  */
#line 2542 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2((yyvsp[-3].i), SPO_SEL_GROW));
		  }
#line 5232 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 374: /* ter_selection_x: filter_ID '(' SPERCENT ',' ter_selection ')'  */
#line 2546 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iio",
			     VA_PASS3((yyvsp[-3].i), SPOFILTER_PERCENT, SPO_SEL_FILTER));
		  }
#line 5241 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 375: /* ter_selection_x: filter_ID '(' ter_selection ',' ter_selection ')'  */
#line 2551 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
			       VA_PASS2(SPOFILTER_SELECTION, SPO_SEL_FILTER));
		  }
#line 5250 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 376: /* ter_selection_x: filter_ID '(' mapchar_or_var ',' ter_selection ')'  */
#line 2556 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io",
				 VA_PASS2(SPOFILTER_MAPCHAR, SPO_SEL_FILTER));
		  }
#line 5259 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 377: /* ter_selection_x: flood_ID coord_or_var  */
#line 2561 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_FLOOD));
		  }
#line 5267 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 378: /* ter_selection_x: circle_ID '(' coord_or_var ',' math_expr_var ')'  */
#line 2565 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "oio",
				 VA_PASS3(SPO_COPY, 1, SPO_SEL_ELLIPSE));
		  }
#line 5276 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 379: /* ter_selection_x: circle_ID '(' coord_or_var ',' math_expr_var ',' FILLING ')'  */
#line 2570 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "oio",
				 VA_PASS3(SPO_COPY, (yyvsp[-1].i), SPO_SEL_ELLIPSE));
		  }
#line 5285 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 380: /* ter_selection_x: ellipse_ID '(' coord_or_var ',' math_expr_var ',' math_expr_var ')'  */
#line 2575 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2(1, SPO_SEL_ELLIPSE));
		  }
#line 5293 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 381: /* ter_selection_x: ellipse_ID '(' coord_or_var ',' math_expr_var ',' math_expr_var ',' FILLING ')'  */
#line 2579 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "io", VA_PASS2((yyvsp[-1].i), SPO_SEL_ELLIPSE));
		  }
#line 5301 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 382: /* ter_selection_x: gradient_ID '(' GRADIENT_TYPE ',' '(' math_expr_var '-' math_expr_var opt_limited ')' ',' coord_or_var opt_coord_or_var ')'  */
#line 2583 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iio",
				 VA_PASS3((yyvsp[-5].i), (yyvsp[-11].i), SPO_SEL_GRADIENT));
		  }
#line 5310 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 383: /* ter_selection_x: complement_ID ter_selection_x  */
#line 2588 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_COMPLEMENT));
		  }
#line 5318 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 384: /* ter_selection_x: VARSTRING_SEL  */
#line 2592 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      check_vardef_type(vardefs, (yyvsp[0].map), SPOVAR_SEL);
		      vardef_used(vardefs, (yyvsp[0].map));
		      add_opvars(splev, "v", VA_PASS1((yyvsp[0].map)));
		      Free((yyvsp[0].map));
		  }
#line 5329 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 385: /* ter_selection_x: '(' ter_selection ')'  */
#line 2599 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 5337 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 386: /* ter_selection: ter_selection_x  */
#line 2605 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 5345 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 387: /* ter_selection: ter_selection_x '&' ter_selection  */
#line 2609 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "o", VA_PASS1(SPO_SEL_ADD));
		  }
#line 5353 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 388: /* dice: DICE  */
#line 2615 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "iio",
				 VA_PASS3((yyvsp[0].dice).num, (yyvsp[0].dice).die, SPO_DICE));
		  }
#line 5362 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 392: /* all_ints_push: MINUS_INTEGER  */
#line 2627 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1((yyvsp[0].i)));
		  }
#line 5370 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 393: /* all_ints_push: PLUS_INTEGER  */
#line 2631 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1((yyvsp[0].i)));
		  }
#line 5378 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 394: /* all_ints_push: INTEGER  */
#line 2635 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      add_opvars(splev, "i", VA_PASS1((yyvsp[0].i)));
		  }
#line 5386 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 395: /* all_ints_push: dice  */
#line 2639 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
		      /* nothing */
		  }
#line 5394 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 404: /* lev_region: region  */
#line 2661 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			(yyval.lregn) = (yyvsp[0].lregn);
		  }
#line 5402 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 405: /* lev_region: LEV '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'  */
#line 2665 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
			if ((yyvsp[-7].i) <= 0 || (yyvsp[-7].i) >= COLNO)
			    lc_error(
                          "Region (%ld,%ld,%ld,%ld) out of level range (x1)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			else if ((yyvsp[-5].i) < 0 || (yyvsp[-5].i) >= ROWNO)
			    lc_error(
                          "Region (%ld,%ld,%ld,%ld) out of level range (y1)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			else if ((yyvsp[-3].i) <= 0 || (yyvsp[-3].i) >= COLNO)
			    lc_error(
                          "Region (%ld,%ld,%ld,%ld) out of level range (x2)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			else if ((yyvsp[-1].i) < 0 || (yyvsp[-1].i) >= ROWNO)
			    lc_error(
                          "Region (%ld,%ld,%ld,%ld) out of level range (y2)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			(yyval.lregn).x1 = (yyvsp[-7].i);
			(yyval.lregn).y1 = (yyvsp[-5].i);
			(yyval.lregn).x2 = (yyvsp[-3].i);
			(yyval.lregn).y2 = (yyvsp[-1].i);
			(yyval.lregn).area = 1;
		  }
#line 5430 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;

  case 406: /* region: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'  */
#line 2691 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"
                  {
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if ((yyvsp[-7].i) < 0 || (yyvsp[-7].i) > (int) max_x_map)
			    lc_error(
                            "Region (%ld,%ld,%ld,%ld) out of map range (x1)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			else if ((yyvsp[-5].i) < 0 || (yyvsp[-5].i) > (int) max_y_map)
			    lc_error(
                            "Region (%ld,%ld,%ld,%ld) out of map range (y1)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			else if ((yyvsp[-3].i) < 0 || (yyvsp[-3].i) > (int) max_x_map)
			    lc_error(
                            "Region (%ld,%ld,%ld,%ld) out of map range (x2)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			else if ((yyvsp[-1].i) < 0 || (yyvsp[-1].i) > (int) max_y_map)
			    lc_error(
                            "Region (%ld,%ld,%ld,%ld) out of map range (y2)!",
                                     (yyvsp[-7].i), (yyvsp[-5].i), (yyvsp[-3].i), (yyvsp[-1].i));
			(yyval.lregn).area = 0;
			(yyval.lregn).x1 = (yyvsp[-7].i);
			(yyval.lregn).y1 = (yyvsp[-5].i);
			(yyval.lregn).x2 = (yyvsp[-3].i);
			(yyval.lregn).y2 = (yyvsp[-1].i);
		  }
#line 5460 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"
    break;


#line 5464 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_parser.c"

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

#line 2719 "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util/lev_comp.y"


/*lev_comp.y*/
