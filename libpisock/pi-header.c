/*
 * pi-header.c:  Splash for the version/etc. 
 *
 * Copyright (c) 2000, David A. Desrosiers
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "pi-version.h"

void display_splash(char *progname);

void display_splash(char *progname)
{
        char 	*patchlevel = "";

#ifdef PILOT_LINK_PATCH
        patchlevel = PILOT_LINK_PATCH;
#endif
	printf("   .--------------------------------------------.\n"
	       "   | (c) Copyright 1996-2003, pilot-link team   |\n"
	       "   |   Join the pilot-link lists to help out.   |\n"
	       "   `--------------------------------------------'\n"
	       "   This is %s, from pilot-link version %d.%d.%d%s\n\n"
	       "   Build target..: %s\n"
	       "   Build date....: %s %s\n\n", progname, 
		PILOT_LINK_VERSION, PILOT_LINK_MAJOR, PILOT_LINK_MINOR, 
		patchlevel, HOST_OS, __DATE__, __TIME__);

	printf("   pilot-link %d.%d.%d%s is covered under the GPL/LGPL\n",
                PILOT_LINK_VERSION, PILOT_LINK_MAJOR, PILOT_LINK_MINOR,
                patchlevel);

	printf("   See the file COPYING under docs for more info.\n\n"
	       "   Please use -h for more detailed options.\n");

}
