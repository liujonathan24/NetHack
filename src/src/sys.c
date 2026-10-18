/* NetHack 3.6	sys.c	$NHDT-Date: 1575665952 2019/12/06 20:59:12 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.46 $ */
/* Copyright (c) Kenneth Lorber, Kensington, Maryland, 2008. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifndef SYSCF
/* !SYSCF configurations need '#define DEBUGFILES "foo.c bar.c"'
 * to enable debugging feedback for source files foo.c and bar.c;
 * to activate debugpline(), set an appropriate value and uncomment
 */
/* # define DEBUGFILES "*" */

/* note: DEBUGFILES value here or in sysconf.DEBUGFILES can be overridden
   at runtime by setting up a value for "DEBUGFILES" in the environment */
#endif

/* sysopt: per-env nh_g->sysopt */

void
sys_early_init()
{
    NH_G(sysopt).support = (char *) 0;
    NH_G(sysopt).recover = (char *) 0;
#ifdef SYSCF
    sysopt.wizards = (char *) 0;
#else
    NH_G(sysopt).wizards = dupstr(WIZARD_NAME);
#endif
#if defined(SYSCF) || !defined(DEBUGFILES)
    NH_G(sysopt).debugfiles = (char *) 0;
#else
    sysopt.debugfiles = dupstr(DEBUGFILES);
#endif
#ifdef DUMPLOG
    sysopt.dumplogfile = (char *) 0;
#endif
    NH_G(sysopt).env_dbgfl = 0; /* haven't checked getenv("DEBUGFILES") yet */
    NH_G(sysopt).shellers = (char *) 0;
    NH_G(sysopt).explorers = (char *) 0;
    NH_G(sysopt).genericusers = (char *) 0;
    NH_G(sysopt).maxplayers = 0; /* XXX eventually replace MAX_NR_OF_PLAYERS */
    NH_G(sysopt).bones_pools = 0;

    /* record file */
    NH_G(sysopt).persmax = PERSMAX;
    NH_G(sysopt).entrymax = ENTRYMAX;
    NH_G(sysopt).pointsmin = POINTSMIN;
    NH_G(sysopt).pers_is_uid = PERS_IS_UID;
    NH_G(sysopt).tt_oname_maxrank = 10;

    /* sanity checks */
    if (PERSMAX < 1)
        NH_G(sysopt).persmax = 1;
    if (ENTRYMAX < 10)
        NH_G(sysopt).entrymax = 10;
    if (POINTSMIN < 1)
        NH_G(sysopt).pointsmin = 1;
    if (PERS_IS_UID != 0 && PERS_IS_UID != 1)
        panic("config error: PERS_IS_UID must be either 0 or 1");

#ifdef PANICTRACE
    /* panic options */
    NH_G(sysopt).gdbpath = dupstr(GDBPATH);
    NH_G(sysopt).greppath = dupstr(GREPPATH);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    NH_G(sysopt).panictrace_gdb = 1;
#ifdef PANICTRACE_LIBC
    NH_G(sysopt).panictrace_libc = 2;
#endif
#else
    sysopt.panictrace_gdb = 0;
#ifdef PANICTRACE_LIBC
    sysopt.panictrace_libc = 0;
#endif
#endif
#endif

    NH_G(sysopt).check_save_uid = 1;
    NH_G(sysopt).check_plname = 0;
    NH_G(sysopt).seduce = 1; /* if it's compiled in, default to on */
    sysopt_seduce_set(NH_G(sysopt).seduce);
    NH_G(sysopt).accessibility = 0;
#ifdef WIN32
    sysopt.portable_device_paths = 0;
#endif
    return;
}

void
sysopt_release()
{
    if (NH_G(sysopt).support)
        free((genericptr_t) NH_G(sysopt).support), NH_G(sysopt).support = (char *) 0;
    if (NH_G(sysopt).recover)
        free((genericptr_t) NH_G(sysopt).recover), NH_G(sysopt).recover = (char *) 0;
    if (NH_G(sysopt).wizards)
        free((genericptr_t) NH_G(sysopt).wizards), NH_G(sysopt).wizards = (char *) 0;
    if (NH_G(sysopt).explorers)
        free((genericptr_t) NH_G(sysopt).explorers), NH_G(sysopt).explorers = (char *) 0;
    if (NH_G(sysopt).shellers)
        free((genericptr_t) NH_G(sysopt).shellers), NH_G(sysopt).shellers = (char *) 0;
    if (NH_G(sysopt).debugfiles)
        free((genericptr_t) NH_G(sysopt).debugfiles),
        NH_G(sysopt).debugfiles = (char *) 0;
#ifdef DUMPLOG
    if (sysopt.dumplogfile)
        free((genericptr_t)sysopt.dumplogfile), sysopt.dumplogfile=(char *)0;
#endif
    if (NH_G(sysopt).genericusers)
        free((genericptr_t) NH_G(sysopt).genericusers),
        NH_G(sysopt).genericusers = (char *) 0;
    if (NH_G(sysopt).gdbpath)
        free((genericptr_t) NH_G(sysopt).gdbpath), NH_G(sysopt).gdbpath = (char *) 0;
    if (NH_G(sysopt).greppath)
        free((genericptr_t) NH_G(sysopt).greppath), NH_G(sysopt).greppath = (char *) 0;

    /* this one's last because it might be used in panic feedback, although
       none of the preceding ones are likely to trigger a controlled panic */
    if (NH_G(sysopt).fmtd_wizard_list)
        free((genericptr_t) NH_G(sysopt).fmtd_wizard_list),
        NH_G(sysopt).fmtd_wizard_list = (char *) 0;
    return;
}

extern const struct attack sa_yes[NATTK];
extern const struct attack sa_no[NATTK];

void
sysopt_seduce_set(val)
int val;
{
#if 0
/*
 * Attack substitution is now done on the fly in getmattk(mhitu.c).
 */
    struct attack *setval = val ? sa_yes : sa_no;
    int x;

    for (x = 0; x < NATTK; x++) {
        mons[PM_INCUBUS].mattk[x] = setval[x];
        mons[PM_SUCCUBUS].mattk[x] = setval[x];
    }
#else
    nhUse(val);
#endif /*0*/
    return;
}

/*sys.c*/
