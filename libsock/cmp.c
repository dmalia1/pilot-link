/*
 * cmp.c:  Pilot CMP protocol
 *
 * Copyright (c) 1996, Kenneth Albanowski.
 * Copyright (c) 1999, Tilo Christ
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
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#include <stdio.h>

#include "pi-debug.h"
#include "pi-source.h"
#include "pi-socket.h"
#include "pi-padp.h"
#include "pi-cmp.h"
#include "pi-serial.h"


/***********************************************************************
 *
 * Function:    cmp_rx
 *
 * Summary:     Receive CMP packets
 *
 * Parmeters:   None
 *
 * Returns:     A negative number on error, 0 otherwise
 *
 ***********************************************************************/
int cmp_rx(struct pi_socket *ps, struct cmp *c)
{
	int l;
	unsigned char cmpbuf[10];
 
	Begin(cmp_rx);

	l = padp_rx(ps, cmpbuf, 10);

	if (l < 10)
		return -1;

	CHECK(PI_DBG_CMP, PI_DBG_LVL_INFO, cmp_dump(cmpbuf, 0));

	c->type = get_byte(cmpbuf);
	c->flags = get_byte(cmpbuf + 1);
	c->version = get_short(cmpbuf + 2);
	c->reserved = get_short(cmpbuf + 4);
	c->baudrate = get_long(cmpbuf + 6);

	End(cmp_rx);

	return 0;
}

/***********************************************************************
 *
 * Function:    cmp_init
 *
 * Summary:     Initialize the socket for CMP transmission
 *
 * Parmeters:   None
 *
 * Returns:     Number of packets transmitted
 *
 ***********************************************************************/
int cmp_init(struct pi_socket *ps, int baudrate)
{	
	unsigned char cmpbuf[10];
	int type, size;
	
	set_byte(cmpbuf + 0, 2);
	set_long(cmpbuf + 2, 0);
	set_long(cmpbuf + 6, baudrate);

	if (baudrate != 9600)
		set_byte(cmpbuf + 1, 0x80);
	else
		set_byte(cmpbuf + 1, 0);

	CHECK(PI_DBG_CMP, PI_DBG_LVL_INFO, cmp_dump(cmpbuf, 1));

	type = padData;
	size = sizeof(type);
	pi_setsockopt(ps->sd, PI_LEVEL_PADP, PI_PADP_TYPE, 
		      &type, &size);

	return padp_tx(ps, cmpbuf, 10);
}

/***********************************************************************
 *
 * Function:    cmp_abort
 *
 * Summary:     Abort a CMP session in progress
 *
 * Parmeters:   None
 *
 * Returns:     Number of PADP packets transmitted
 *
 ***********************************************************************/
int cmp_abort(struct pi_socket *ps, int reason)
{
	unsigned char cmpbuf[10];
	int type, size;
	
	set_byte(cmpbuf + 0, 3);
	set_byte(cmpbuf + 1, reason);
	set_long(cmpbuf + 2, 0);
	set_long(cmpbuf + 6, 0);

	CHECK(PI_DBG_CMP, PI_DBG_LVL_INFO, cmp_dump(cmpbuf, 1));

	type = padData;
	size = sizeof(type);
	pi_setsockopt(ps->sd, PI_LEVEL_PADP, PI_PADP_TYPE, 
		      &type, &size);

	return padp_tx(ps, cmpbuf, 10);
}

/***********************************************************************
 *
 * Function:    cmp_wakeup
 *
 * Summary:     Wakeup the CMP listener process
 *
 * Parmeters:   None
 *
 * Returns:     Number of PADP packets transmitted
 *
 ***********************************************************************/
int cmp_wakeup(struct pi_socket *ps, int maxbaud)
{
	unsigned char cmpbuf[200];
	int type, size;
	
	set_byte(cmpbuf + 0, 1);
	set_byte(cmpbuf + 1, 0);
	set_short(cmpbuf + 2, CommVersion_1_0);
	set_short(cmpbuf + 4, 0);
	set_long(cmpbuf + 6, maxbaud);

	CHECK(PI_DBG_CMP, PI_DBG_LVL_INFO, cmp_dump(cmpbuf, 1));

	type = padWake;
	size = sizeof(type);
	pi_setsockopt(ps->sd, PI_LEVEL_PADP, PI_PADP_TYPE, 
		      &type, &size);

	return padp_tx(ps, cmpbuf, 10);
}

/***********************************************************************
 *
 * Function:    cmp_dump
 *
 * Summary:     Dump the CMP packet frames
 *
 * Parmeters:   None
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
void cmp_dump(unsigned char *cmp, int rxtx)
{
	LOG(PI_DBG_CMP, PI_DBG_LVL_NONE,
	    "CMP %s %s", rxtx ? "TX" : "RX",
	    (get_byte(cmp) == 1) ? "WAKE"
	    : (get_byte(cmp) == 2) ? "INIT"
	    : (get_byte(cmp) == 3) ? "ABRT" : "");
	if ((get_byte(cmp) < 1) || (get_byte(cmp) > 3))
		LOG(PI_DBG_CMP, PI_DBG_LVL_NONE, "UNK %d", get_byte(cmp));
	LOG(PI_DBG_CMP, PI_DBG_LVL_NONE,
	    "  Type: %2.2X Flags: %2.2X Version: %8.8lX Baud: %8.8lX (%ld)\n",
	    get_byte(cmp), get_byte(cmp + 1), get_long(cmp + 2),
	    get_long(cmp + 6), get_long(cmp + 6));
}
