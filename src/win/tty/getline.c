/* NetHack 3.6	getline.c	$NHDT-Date: 1543830347 2018/12/03 09:45:47 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.37 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Copyright (c) Michael Allison, 2006. */
/* Copyright (c) Facebook, Inc. and its affiliates. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "nle.h" /* Current_nle_ctx */

#ifdef TTY_GRAPHICS

#if !defined(MAC)
#define NEWAUTOCOMP
#endif

#include "wintty.h"
#include "func_tab.h"

/* Morc migrated to nle_ctx_t (stage 10'). Macro in wintty.h. */
/* Suppress_history was a plain STATIC_VAR (process-global).
 * Concurrent OMP envs at different stages of tty_getlin/ext_cmd_getlin_hook
 * could race: env A sets FALSE, env B sees FALSE and reads the wrong history
 * suppress state. Now per-env via nle_ctx_t. */
#define suppress_history (current_nle_ctx->s_suppress_history)
STATIC_DCL boolean FDECL(ext_cmd_getlin_hook, (char *));

typedef boolean FDECL((*getlin_hook_proc), (char *));

STATIC_DCL void FDECL(hooked_tty_getlin,
                      (const char *, char *, getlin_hook_proc));
extern int NDECL(extcmd_via_menu); /* Cmd.c */

extern char erase_char, kill_char; /* From appropriate tty.c file */

/*
 * Read a line closed with '\n' into the array char bufp[BUFSZ].
 * (The '\n' is not stored. The string is closed with a '\0'.)
 * Reading can be interrupted by an escape ('\033') - now the
 * resulting string is "\033".
 */
void
tty_getlin(query, bufp)
const char *query;
register char *bufp;
{
    suppress_history = FALSE;
    hooked_tty_getlin(query, bufp, (getlin_hook_proc) 0);
}

STATIC_OVL void
hooked_tty_getlin(query, bufp, hook)
const char *query;
register char *bufp;
getlin_hook_proc hook;
{
    register char *obufp = bufp;
    register int c;
    struct WinDesc *cw = wins[WIN_MESSAGE];
    boolean doprev = 0;

    if (ttyDisplay->toplin == 1 && !(cw->wflags & WIN_STOP))
        more();
    cw->wflags &= ~WIN_STOP;
    ttyDisplay->toplin = 3; /* Special prompt state */
    ttyDisplay->inread++;

    /* Issue the prompt */
    custompline(OVERRIDE_MSGTYPE | SUPPRESS_HISTORY, "%s ", query);
#ifdef EDIT_GETLIN
    /* Bufp is input/output; treat current contents (presumed to be from
       previous getlin()) as default input */
    addtopl(obufp);
    bufp = eos(obufp);
#else
    /* !EDIT_GETLIN: bufp is output only; init it to empty */
    *bufp = '\0';
#endif

    for (;;) {
        (void) fflush(stdout);
        Strcat(strcat(strcpy(toplines, query), " "), obufp);
        c = pgetchar();
        if (c == '\033' || c == EOF) {
            if (c == '\033' && obufp[0] != '\0') {
                obufp[0] = '\0';
                bufp = obufp;
                tty_clear_nhwindow(WIN_MESSAGE);
                cw->maxcol = cw->maxrow;
                addtopl(query);
                addtopl(" ");
                addtopl(obufp);
            } else {
                obufp[0] = '\033';
                obufp[1] = '\0';
                break;
            }
        }
        if (ttyDisplay->intr) {
            ttyDisplay->intr--;
            *bufp = 0;
        }
        if (c == '\020') { /* Ctrl-P */
            if (iflags.prevmsg_window != 's') {
                int sav = ttyDisplay->inread;

                ttyDisplay->inread = 0;
                (void) tty_doprev_message();
                ttyDisplay->inread = sav;
                tty_clear_nhwindow(WIN_MESSAGE);
                cw->maxcol = cw->maxrow;
                addtopl(query);
                addtopl(" ");
                *bufp = 0;
                addtopl(obufp);
            } else {
                if (!doprev)
                    (void) tty_doprev_message(); /* Need two initially */
                (void) tty_doprev_message();
                doprev = 1;
                continue;
            }
        } else if (doprev && iflags.prevmsg_window == 's') {
            tty_clear_nhwindow(WIN_MESSAGE);
            cw->maxcol = cw->maxrow;
            doprev = 0;
            addtopl(query);
            addtopl(" ");
            *bufp = 0;
            addtopl(obufp);
        }
        if (c == erase_char || c == '\b') {
            if (bufp != obufp) {
#ifdef NEWAUTOCOMP
                char *i;

#endif /* NEWAUTOCOMP */
                bufp--;
#ifndef NEWAUTOCOMP
                putsyms("\b \b"); /* Putsym converts \b */
#else                             /* NEWAUTOCOMP */
                putsyms("\b");
                for (i = bufp; *i; ++i)
                    putsyms(" ");
                for (; i > bufp; --i)
                    putsyms("\b");
                *bufp = 0;
#endif                            /* NEWAUTOCOMP */
            } else
                tty_nhbell();
        } else if (c == '\n' || c == '\r') {
#ifndef NEWAUTOCOMP
            *bufp = 0;
#endif /* Not NEWAUTOCOMP */
            break;
        } else if (' ' <= (unsigned char) c && c != '\177'
                   /* Avoid isprint() - some people don't have it
                      ' ' is not always a printing char */
                   && (bufp - obufp < BUFSZ - 1 && bufp - obufp < COLNO)) {
#ifdef NEWAUTOCOMP
            char *i = eos(bufp);

#endif /* NEWAUTOCOMP */
            *bufp = c;
            bufp[1] = 0;
            putsyms(bufp);
            bufp++;
            if (hook && (*hook)(obufp)) {
                putsyms(bufp);
#ifndef NEWAUTOCOMP
                bufp = eos(bufp);
#else  /* NEWAUTOCOMP */
                /* Pointer and cursor left where they were */
                for (i = bufp; *i; ++i)
                    putsyms("\b");
            } else if (i > bufp) {
                char *s = i;

                /* Erase rest of prior guess */
                for (; i > bufp; --i)
                    putsyms(" ");
                for (; s > bufp; --s)
                    putsyms("\b");
#endif /* NEWAUTOCOMP */
            }
        } else if (c == kill_char || c == '\177') { /* Robert Viduya */
            /* This test last - @ might be the kill_char */
#ifndef NEWAUTOCOMP
            while (bufp != obufp) {
                bufp--;
                putsyms("\b \b");
            }
#else  /* NEWAUTOCOMP */
            for (; *bufp; ++bufp)
                putsyms(" ");
            for (; bufp != obufp; --bufp)
                putsyms("\b \b");
            *bufp = 0;
#endif /* NEWAUTOCOMP */
        } else
            tty_nhbell();
    }
    ttyDisplay->toplin = 2; /* Nonempty, no --More-- required */
    ttyDisplay->inread--;
    clear_nhwindow(WIN_MESSAGE); /* Clean up after ourselves */

    if (suppress_history) {
        /* Prevent next message from pushing current query+answer into
           tty message history */
        *toplines = '\0';
#ifdef DUMPLOG
    } else {
        /* Needed because we've bypassed pline() */
        dumplogmsg(toplines);
#endif
    }
}

/*
 * Hack for RL window proc: register if we are in xwaitforspace context.
 */
/* xwaitingforspace — migrated to nle_ctx_t. */
#define xwaitingforspace (current_nle_ctx->xwaitingforspace_v)

void
xwaitforspace(s)
register const char *s; /* Chars allowed besides return */
{
    register int c, x = ttyDisplay ? (int) ttyDisplay->dismiss_more : '\n';

    xwaitingforspace = TRUE;
    morc = 0;
    while (
#ifdef HANGUPHANDLING
        !current_nle_ctx->program_state.done_hup &&
#endif
        (c = nhgetch()) != EOF) {
        if (c == '\n' || c == '\r')
            break;

        if (iflags.cbreak) {
            if (c == '\033') {
                if (ttyDisplay)
                    ttyDisplay->dismiss_more = 1;
                morc = '\033';
                break;
            }
            if ((s && index(s, c)) || c == x || (x == '\n' && c == '\r')) {
                morc = (char) c;
                break;
            }
            tty_nhbell();
        }
    }
    xwaitingforspace = FALSE;
}

/*
 * Implement extended command completion by using this hook into
 * tty_getlin.  Check the characters already typed, if they uniquely
 * identify an extended command, expand the string to the whole
 * command.
 *
 * Return TRUE if we've extended the string at base.  Otherwise return FALSE.
 * Assumptions:
 *
 *	+ we don't change the characters that are already in base
 *	+ base has enough room to hold our string
 */
STATIC_OVL boolean
ext_cmd_getlin_hook(base)
char *base;
{
    int oindex, com_index;

    com_index = -1;
    for (oindex = 0; extcmdlist[oindex].ef_txt != (char *) 0; oindex++) {
        if (extcmdlist[oindex].cmd_flags & CMD_NOT_AVAILABLE)
            continue;
        if ((extcmdlist[oindex].cmd_flags & AUTOCOMPLETE)
            && !(!wizard && (extcmdlist[oindex].cmd_flags & WIZMODECMD))
            && !strncmpi(base, extcmdlist[oindex].ef_txt, strlen(base))) {
            if (com_index == -1) /* No matches yet */
                com_index = oindex;
            else /* More than 1 match */
                return FALSE;
        }
    }
    if (com_index >= 0) {
        Strcpy(base, extcmdlist[com_index].ef_txt);
        return TRUE;
    }

    return FALSE; /* Didn't match anything */
}

/*
 * Read in an extended command, doing command line completion.  We
 * stop when we have found enough characters to make a unique command.
 */
int
tty_get_ext_cmd()
{
    int i;
    char buf[BUFSZ];

    if (iflags.extmenu)
        return extcmd_via_menu();

    suppress_history = TRUE;
    /* Maybe a runtime option?
     * hooked_tty_getlin("#", buf,
     *                   (flags.cmd_comp && !in_doagain)
     *                      ? ext_cmd_getlin_hook
     *                      : (getlin_hook_proc) 0);
     */
    buf[0] = '\0';
    hooked_tty_getlin("#", buf, in_doagain ? (getlin_hook_proc) 0
                                           : ext_cmd_getlin_hook);
    (void) mungspaces(buf);
    if (buf[0] == 0 || buf[0] == '\033')
        return -1;

    for (i = 0; extcmdlist[i].ef_txt != (char *) 0; i++)
        if (!strcmpi(buf, extcmdlist[i].ef_txt))
            break;

    if (!in_doagain) {
        int j;
        for (j = 0; buf[j]; j++)
            savech(buf[j]);
        savech('\n');
    }

    if (extcmdlist[i].ef_txt == (char *) 0) {
        pline("%s: unknown extended command.", buf);
        i = -1;
    }

    return i;
}

#endif /* TTY_GRAPHICS */

/*getline.c*/
