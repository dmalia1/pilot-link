/*
 * userland.h: General definitions for userland conduits.
 *
 * Copyright (C) 2004 by Adriaan de Groot <groot@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifndef PALM_USERLAND_H
#define PALM_USERLAND_H

#include <popt.h>
#include "pi-appinfo.h"

/*
 * This file defines general stuff for conduits -- common option processing,
 * perhaps some utility functions, etc. It prescribes how some of the code
 * of a conduit should look like, so as to preserve uniformity of
 * options and handling.
 *
 * - Each conduit should start its options table with USERLAND_RESERVED_OPTIONS.
 *   This sets up the standard options --port, --version, --quiet as well as
 *   popt autohelp.
 * - If an error is found while processing options, call userland_badoption.
 *   This produces a standard error message.
 * - If no error is found, call userland_connect() instead of pilot_connect().
 *   This does the same as pilot_connect, but obeys --quiet and produces
 *   output on stderr only if there _is_ an error.
 */


/*
 * These are definitions for popt support in userland. Every conduit's
 * popt table should start with USERLAND_RESERVED_OPTIONS to insert
 * the standard options into it. Also enables autohelp.
 */

#define USERLAND_RESERVED_OPTIONS \
	{NULL,0,POPT_ARG_INCLUDE_TABLE,userland_common_options,0,"Options common to all conduits.",NULL}, \
	POPT_AUTOHELP


/*
 * Connect to the Pilot specified by any --port option, respecting
 * the quiet flag as well. This is basically pilot_connect(), but
 * marginally cleaner.
 */

extern int userland_connect();

/*
 * Complain about a bad option and exit();
 */

extern void userland_badoption(poptContext pc, int optc);

/*
 * Look up a category name. Argument @p info is the category part of the
 * AppInfo block for the database, while @p name is the category to look up.
 * Returns the index of the category if found (0..15) or -1 if not.
 */

extern int userland_findcategory(const struct CategoryAppInfo *info, const char *name);

/*
 * Add an alias to a popt context; remember to use --bad-option in the alias
 * to add a complaint about deprecated options. Do not pass in both
 * a long and a short option in one go, use two calls for that.
 */
void userland_popt_alias(poptContext pc,
	const char *alias_long,
	char alias_short,
	const char *expansion);

/*
 * Set explanation of what options to use in response to an alias
 * that contains --bad-option.
 */
void userland_set_badoption_help(const char *help);

/*
 * We need to be able to refer to the table of common options.
 */

extern struct poptOption userland_common_options[];
extern int userland_quiet;


#endif

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */