// powers.cpp -- Power manipulation routines.
//
// $Id: powers.cpp,v 1.17 2006/08/09 15:45:14 sdennis Exp $
//

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"

#include "command.h"
#include "powers.h"

/* ---------------------------------------------------------------------------
 * ph_any: set or clear indicated bit, no security checking
 */

static bool ph_any(dbref target, dbref player, POWER power, int fpowers, bool reset)
{
    UNUSED_PARAMETER(player);

    if (fpowers & POWER_EXT)
    {
        if (reset)
        {
            s_Powers2(target, Powers2(target) & ~power);
        }
        else
        {
            s_Powers2(target, Powers2(target) | power);
        }
    }
    else
    {
        if (reset)
        {
            s_Powers(target, Powers(target) & ~power);
        }
        else
        {
            s_Powers(target, Powers(target) | power);
        }
    }
    return true;
}

/* ---------------------------------------------------------------------------
 * ph_god: only GOD may set or clear the bit
 */

static bool ph_god(dbref target, dbref player, POWER power, int fpowers, bool reset)
{
    if (!God(player))
    {
        return false;
    }
    return (ph_any(target, player, power, fpowers, reset));
}

/* ---------------------------------------------------------------------------
 * ph_wiz: only WIZARDS (or GOD) may set or clear the bit
 */

static bool ph_wiz(dbref target, dbref player, POWER power, int fpowers, bool reset)
{
    if (!Wizard(player))
    {
        return false;
    }
    return (ph_any(target, player, power, fpowers, reset));
}

#if 0

/* ---------------------------------------------------------------------------
 * ph_wizroy: only WIZARDS, ROYALTY, (or GOD) may set or clear the bit
 */

bool ph_wizroy(dbref target, dbref player, POWER power, int fpowers, bool reset)
{
    if (!WizRoy(player))
    {
        return false;
    }
    return (ph_any(target, player, power, fpowers, reset));
}

/* ---------------------------------------------------------------------------
 * ph_inherit: only players may set or clear this bit.
 */

bool ph_inherit(dbref target, dbref player, POWER power, int fpowers, bool reset)
{
    if (!Inherits(player))
    {
        return false;
    }
    return (ph_any(target, player, power, fpowers, reset));
}
#endif

static POWERENT gen_powers[] =
{
    {"announce",        POW_ANNOUNCE,   0, 0,   ph_wiz},
    {"boot",            POW_BOOT,       0, 0,   ph_wiz},
#if defined(FIRANMUX)
    {"builder",         POW_BUILDER,    POWER_EXT,  0,  ph_any},
#else
    {"builder",         POW_BUILDER,    POWER_EXT,  0,  ph_wiz},
#endif // FIRANMUX
    {"chown_anything",  POW_CHOWN_ANY,  0, 0,   ph_wiz},
    {"comm_all",        POW_COMM_ALL,   0, 0,   ph_wiz},
    {"control_all",     POW_CONTROL_ALL,0, 0,   ph_god},
    {"expanded_who",    POW_WIZARD_WHO, 0, 0,   ph_wiz},
    {"find_unfindable", POW_FIND_UNFIND,0, 0,   ph_wiz},
    {"free_money",      POW_FREE_MONEY, 0, 0,   ph_wiz},
    {"free_quota",      POW_FREE_QUOTA, 0, 0,   ph_wiz},
    {"guest",           POW_GUEST,      0, 0,   ph_god},
    {"halt",            POW_HALT,       0, 0,   ph_wiz},
#if defined(FIRANMUX)
    {"hide",            POW_HIDE,       0, 0,   ph_any},
#else
    {"hide",            POW_HIDE,       0, 0,   ph_wiz},
#endif // FIRANMUX
    {"idle",            POW_IDLE,       0, 0,   ph_wiz},
#if defined(FIRANMUX)
    {"immutable",       POW_IMMUTABLE,  POWER_EXT, 0, ph_wiz},
#endif // FIRANMUX
    {"long_fingers",    POW_LONGFINGERS,0, 0,   ph_wiz},
#if defined(FIRANMUX)
    {"monitor",         POW_MONITOR,    0, 0,   ph_any},
#else
    {"monitor",         POW_MONITOR,    0, 0,   ph_wiz},
#endif // FIRANMUX
    {"no_destroy",      POW_NO_DESTROY, 0, 0,   ph_wiz},
    {"pass_locks",      POW_PASS_LOCKS, 0, 0,   ph_wiz},
    {"poll",            POW_POLL,       0, 0,   ph_wiz},
    {"prog",            POW_PROG,       0, 0,   ph_wiz},
    {"quota",           POW_CHG_QUOTAS, 0, 0,   ph_wiz},
    {"search",          POW_SEARCH,     0, 0,   ph_wiz},
#if defined(FIRANMUX)
    {"see_all",         POW_EXAM_ALL,   0, 0,   ph_any},
#else
    {"see_all",         POW_EXAM_ALL,   0, 0,   ph_wiz},
#endif // FIRANMUX
    {"see_hidden",      POW_SEE_HIDDEN, 0, 0,   ph_wiz},
    {"see_queue",       POW_SEE_QUEUE,  0, 0,   ph_wiz},
    {"siteadmin",       POW_SITEADMIN,  0, 0,   ph_wiz},
    {"stat_any",        POW_STAT_ANY,   0, 0,   ph_wiz},
    {"steal_money",     POW_STEAL,      0, 0,   ph_wiz},
    {"tel_anything",    POW_TEL_UNRST,  0, 0,   ph_wiz},
#if defined(FIRANMUX)
    {"tel_anywhere",    POW_TEL_ANYWHR, 0, 0,   ph_any},
#else
    {"tel_anywhere",    POW_TEL_ANYWHR, 0, 0,   ph_wiz},
#endif // FIRANMUX
    {"unkillable",      POW_UNKILLABLE, 0, 0,   ph_wiz},
    {NULL,              0,              0, 0,   0}
};

/* ---------------------------------------------------------------------------
 * init_powertab: initialize power hash tables.
 */

void init_powertab(void)
{
    POWERENT *fp;
    char *nbuf = alloc_sbuf("init_powertab");

    for (fp = gen_powers; fp->powername; fp++)
    {
        mux_strncpy(nbuf, fp->powername, SBUF_SIZE-1);
        mux_strlwr(nbuf);

        if (!hashfindLEN(nbuf, strlen(nbuf), &mudstate.powers_htab))
        {
            hashaddLEN(nbuf, strlen(nbuf), fp, &mudstate.powers_htab);
        }
    }
    free_sbuf(nbuf);
}

/* ---------------------------------------------------------------------------
 * display_powers: display available powers.
 */

void display_powertab(dbref player)
{
    char *buf, *bp;
    POWERENT *fp;

    bp = buf = alloc_lbuf("display_powertab");
    safe_str("Powers:", buf, &bp);
    for (fp = gen_powers; fp->powername; fp++)
    {
        if ((fp->listperm & CA_WIZARD) && !Wizard(player))
        {
            continue;
        }
        if ((fp->listperm & CA_GOD) && !God(player))
        {
            continue;
        }
        safe_chr(' ', buf, &bp);
        safe_str(fp->powername, buf, &bp);
    }
    *bp = '\0';
    notify(player, buf);
    free_lbuf(buf);
}

static POWERENT *find_power(dbref thing, char *powername)
{
    UNUSED_PARAMETER(thing);

    // Convert powername to canonical lowercase.
    //
    char *buff = alloc_sbuf("find_power");
    mux_strncpy(buff, powername, SBUF_SIZE-1);
    mux_strlwr(buff);
    POWERENT *p = (POWERENT *)hashfindLEN(buff, strlen(buff), &mudstate.powers_htab);
    free_sbuf(buff);
    return p;
}

bool decode_power(dbref player, char *powername, POWERSET *pset)
{
    pset->word1 = 0;
    pset->word2 = 0;

    POWERENT *pent = (POWERENT *)hashfindLEN(powername, strlen(powername), &mudstate.powers_htab);
    if (!pent)
    {
        notify(player, tprintf("%s: Power not found.", powername));
        return false;
    }
    if (pent->powerpower & POWER_EXT)
    {
        pset->word2 = pent->powervalue;
    }
    else
    {
        pset->word1 = pent->powervalue;
    }
    return true;
}

/* ---------------------------------------------------------------------------
 * power_set: Set or clear a specified power on an object.
 */

void power_set(dbref target, dbref player, char *power, int key)
{
    // Trim spaces, and handle the negation character.
    //
    while (mux_isspace(*power))
    {
        power++;
    }

    bool negate = false;
    if (*power == '!')
    {
        negate = true;
        power++;
    }
    while (mux_isspace(*power))
    {
        power++;
    }

    // Make sure a power name was specified.
    //
    if (*power == '\0')
    {
        if (negate)
        {
            notify(player, "You must specify a power to clear.");
        }
        else
        {
            notify(player, "You must specify a power to set.");
        }
        return;
    }
    POWERENT *fp = find_power(target, power);
    if (fp == NULL)
    {
        notify(player, "I don't understand that power.");
        return;
    }

    // Invoke the power handler, and print feedback.
    //
    bool result = fp->handler(target, player, fp->powervalue,
                 fp->powerpower, negate);
    if (!result)
    {
        notify(player, NOPERM_MESSAGE);
    }
    else if (!(key & SET_QUIET) && !Quiet(player))
    {
        notify(player, (negate ? "Cleared." : "Set."));
    }
}

/* ---------------------------------------------------------------------------
 * has_power: does object have power visible to player?
 */

bool has_power(dbref player, dbref it, char *powername)
{
    POWERENT *fp = find_power(it, powername);
    if (!fp)
    {
        return false;
    }

    POWER fv;
    if (fp->powerpower & POWER_EXT)
    {
        fv = Powers2(it);
    }
    else
    {
        fv = Powers(it);
    }

    if (fv & fp->powervalue)
    {
        if ((fp->listperm & CA_WIZARD) && !Wizard(player))
        {
            return false;
        }
        if ((fp->listperm & CA_GOD) && !God(player))
        {
            return false;
        }
        return true;
    }
    return false;
}

/* ---------------------------------------------------------------------------
 * powers_list: Return an LBUG containing the type and powers on thing.
 */

char *powers_list(dbref player, dbref target)
{
    // Allocate the return buffer.
    //
    char *buff = alloc_lbuf("powers_list");
    char *bp = buff;

    bool bFirst = true;
    POWERENT *fp;
    for (fp = gen_powers; fp->powername; fp++)
    {
        POWER fv;
        if (fp->powerpower & POWER_EXT)
        {
            fv = Powers2(target);
        }
        else
        {
            fv = Powers(target);
        }

        if (fv & fp->powervalue)
        {
            if (  (fp->listperm & CA_WIZARD)
               && !Wizard(player))
            {
                continue;
            }
            if (  (fp->listperm & CA_GOD)
               && !God(player))
            {
                continue;
            }
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                safe_chr(' ', buff, &bp);
            }
            safe_str(fp->powername, buff, &bp);
        }
    }

    // Terminate the string, and return the buffer to the caller.
    //
    *bp = '\0';
    return buff;
}


/* ---------------------------------------------------------------------------
 * decompile_powers: Produce commands to set powers on target.
 */

void decompile_powers(dbref player, dbref thing, char *thingname)
{
    POWERENT *fp;

    // Report generic powers.
    //
    POWER f1 = Powers(thing);
    POWER f2 = Powers2(thing);

    for (fp = gen_powers; fp->powername; fp++)
    {
        // Skip if we shouldn't decompile this power
        //
        if (fp->listperm & CA_NO_DECOMP)
        {
            continue;
        }

        // Skip if this power is not set.
        //
        if (fp->powerpower & POWER_EXT)
        {
            if (!(f2 & fp->powervalue))
            {
                continue;
            }
        }
        else
        {
            if (!(f1 & fp->powervalue))
            {
                continue;
            }
        }

        // Skip if we can't see this power.
        //
        if (!check_access(player, fp->listperm))
        {
            continue;
        }

        // We made it this far, report this power.
        //
        notify(player, tprintf("@power %s=%s", strip_ansi(thingname), fp->powername));
    }
}
