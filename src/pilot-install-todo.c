/*
 * install-todo.c:  Palm ToDo list installer
 *
 * Copyright 2002, Martin Fick, based on code in install-todos by Robert A.
 * 		   Kaplan
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

#include "popt.h"
#include <stdio.h>
#include <stdlib.h>

#include "pi-source.h"
#include "pi-dlp.h"
#include "pi-todo.h"
#include "pi-header.h"

poptContext po;

#ifndef HAVE_BASENAME
/* Yes, this isn't efficient... scanning the string twice */
#define basename(s) (strrchr((s), '/') == NULL ? (s) : strrchr((s), '/') + 1)
#endif

#define DATE_STR_MAX 1000

char *strptime(const char *s, const char *format, struct tm *tm);


/***********************************************************************
 *
 * Function:    read_file
 *
 * Summary:     Reads a file from disk to import into ToDoDB.pdb
 *
 * Parameters:  Pointer to a filename
 *
 * Returns:     Text inside the file, of course
 *
 ***********************************************************************/
char *read_file(char *filename)
{
	FILE	*f;
	size_t	filelen;
	char	*file_text 	= NULL;

	f = fopen(filename, "r");
	if (f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	filelen = ftell(f);
	fseek(f, 0, SEEK_SET);

	file_text = (char *) malloc(filelen + 1);
	if (file_text == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	fread(file_text, filelen, 1, f);

	return file_text;
}


/***********************************************************************
 *
 * Function:    install_ToDo
 *
 * Summary:     Adds the entry to ToDoDB.pdb
 *
 * Parameters:  
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
void install_ToDo(int sd, int db, struct ToDo todo)
{
	int 	ToDo_size;

	unsigned char ToDo_buf[0xffff];
	char  duedate[DATE_STR_MAX];

	printf("Indefinite:  %i\n", todo.indefinite);
	if (todo.indefinite == NULL)
		strftime(duedate, DATE_STR_MAX, "%a %b %d %H:%M:%S %Z %Y", &todo.due);
	printf("Due Date:    %s\n", duedate);
	printf("Priority:    %i\n", todo.priority);
	printf("Complete:    %i\n", todo.complete);
	printf("Description: %s\n", todo.description);
	printf("Note:        %s\n", todo.note);

	ToDo_size = pack_ToDo(&todo, ToDo_buf, sizeof(ToDo_buf));

	dlp_WriteRecord(sd, db, 0, 0, 0, ToDo_buf, ToDo_size, 0);

	return;
}

/***********************************************************************
 *
 * Function:    display_help
 *
 * Summary:     Print out the --help options and arguments
 *
 * Parameters:  None
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
static void display_help(const char *progname)
{
	printf("Updates the Palm ToDo list with a single new entry\n\n");

	poptPrintHelp(po, stderr, 0);

	printf("   Examples:\n");
	printf("     %s -p /dev/pilot -n 'Buy Milk' 'Go shopping, see note for items'\n", 
		basename(progname));
	printf("     %s -p /dev/pilot -f ShoppingList.txt 'Go to supermarket'\n\n",
		basename(progname));

        return;
}

int main(int argc, const char *argv[])
{
	int 	sd	= -1,
		db,
		po_err  = -1;

	const char
                *progname 	= argv[0];

	char    *port		= NULL,
		*completed	= NULL,
                *priority	= NULL,
		*due		= NULL,
		*description	= NULL,
		*note		= NULL,
		*filename	= NULL;

	struct 	PilotUser User;
	struct 	ToDo todo;

	todo.indefinite  = 1;

        const struct poptOption options[] = {
                { "port",        'p', POPT_ARG_STRING, &port, 0, "Use device <port> to communicate with Palm", "<port>"},
                { "help",        'h', POPT_ARG_NONE,   0, 'h', "Display this information"},
                { "version",     'v', POPT_ARG_NONE,   0, 'v', "Display version information"},
                { "priority",    'P', POPT_ARG_INT, &priority, 0, "The Priority (default is 1)", "[1|2|3|4|5]"},
                { "due",         'd', POPT_ARG_STRING, &due, 0, "The due Date of the item (default is no date)", "mm/dd/yyyy"},
                { "completed",   'c', POPT_ARG_NONE, &completed, 0, "Mark the item complete (default is incomplete)"},
		{ "description", 'D', POPT_ARG_STRING, &description, 0, "The title of the ToDo entry", "[title]"},
                { "note",        'n', POPT_ARG_STRING, &note, 0, "The Note(tm) text (a single string)", "<note>"},
                { "file",        'f', POPT_ARG_STRING, &filename, 0, "A local filename containing the Note text", "[filename]"},
                POPT_AUTOHELP
                { NULL, 0, 0, NULL, 0 }
        };

        po = poptGetContext("install-todo", argc, (const char **) argv, options, 0);

        if (argc < 2) {
                display_help(progname);
                exit(1);
        }

        while ((po_err = poptGetNextOpt(po)) != -1) {
                switch (po_err) {

		case 'v':
                          print_splash(progname);
                          return 0;
		case 'f': /* Note filename */
			  todo.note = read_file(filename);
			  break;
		default:
		  	display_help(progname);
		  	return 0;
		}
	}

	/*  Setup some todo defaults */
	todo.priority    = priority;
	todo.complete    = completed;
	todo.description = description;
	todo.note        = note;

	if (due != NULL) {
		strptime(due, "%m/%d/%Y", &todo.due);
		todo.indefinite = 0;
		todo.due.tm_sec  = 0;
		todo.due.tm_min  = 0;
		todo.due.tm_hour = 0;
	}

	sd = pilot_connect(port);

        if (sd < 0)
		exit(EXIT_FAILURE);

	/* Open the ToDo database, store access handle in db */
	if (dlp_OpenDB(sd, 0, 0x80 | 0x40, "ToDoDB", &db) < 0) {
		puts("Unable to open ToDo Database");
		dlp_AddSyncLogEntry(sd, "Unable to open ToDo Database.\n");
		exit(EXIT_FAILURE);
	}

	install_ToDo(sd, db, todo);

	dlp_CloseDB(sd, db);

	/* Tell the user who it is, with a different PC id. */
	User.lastSyncPC 	= 0x00010000;
	User.lastSyncDate = User.successfulSyncDate = time(0);
	dlp_WriteUserInfo(sd, &User);

	dlp_AddSyncLogEntry(sd, "Wrote ToDo entry to Palm.\n\n"
		"Thank you for using pilot-link.\n");

	dlp_EndOfSync(sd, 0);
	pi_close(sd);

	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
