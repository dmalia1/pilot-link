/*
 * Pilot Datebook processing utility
 *
 * (c) 2000, Matthias Hessler <pilot-datebook@mhessler.de>
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

#include <string.h>
#include "pilot-datebook-update.h"

/***********************************************************************
 *
 * Function:    update_init
 *
 * Summary:     Initialize update data structure
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
int update_init(struct update_data *updatelist, char *opt_arg)
{
	struct 	update_assign_data *update_assign;
	char 	*pos1, 
		*pos2;

	debug_message("Entering update_init\n");

	updatelist->assign_top 		= NULL;
	updatelist->assign_last 	= NULL;
	updatelist->records_updated 	= 0;

	/* Expected input parameter: update assignments */

	/* Loop over all update assignments, delimited by ';' as delimiter */
	pos1 = opt_arg;

	/* Check for quoted text */
	pos2 = data_find_delimiter(pos1, ';');

	while (pos1 != NULL && *pos1 != '\0') {

		/* Set pos2 properly on end of line */
		if (pos2 == NULL)
			pos2 = pos1 + strlen(pos1);

		/* Alloc update assignments data */
		update_assign = update_new_assign();

		/* Insert assignment data into update assignment list */
		assign_init(&(update_assign->assign), pos1, pos2);
		update_assign->next = NULL;

		/* Insert update assignment into update structure */
		updatelist_assign_add(updatelist, update_assign);

		/* Find next update assignment */
		if (pos2 != NULL && *pos2 != '\0') {
			pos1 = pos2 + 1;
			pos2 = data_find_delimiter(pos1, ';');
		} else {
			break;
		}
	}

	debug_message("Leaving update_init\n");

	return TRUE;
}

/***********************************************************************
 *
 * Function:    update_exit
 *
 * Summary:     Destroy update data structure
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void update_exit(struct update_data *updatelist)
{
	struct update_assign_data *update_assign;

	debug_message("Entering update_exit\n");

	/* Now destroy all update assigns */
	update_assign = updatelist_get_first_assign(updatelist);
	while (update_assign != NULL) {
		debug_message("Removing update assign...\n");

		/* Remove update assign from updatelist */
		updatelist_assign_del(updatelist, update_assign);

		/* Destroy update assign data structure */
		update_free_assign(update_assign);

		/* Next = now first (after removal of previous first) */
		update_assign = updatelist_get_first_assign(updatelist);
	}			/* while */

	/* Init */
	updatelist->assign_top 		= NULL;
	updatelist->assign_last 	= NULL;
	updatelist->records_updated 	= 0;

	debug_message("Leaving update_exit\n");
}



/***********************************************************************
 *
 * Function:    update_set_param
 *
 * Summary:     Set command line options
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
int
update_set_param(struct update_data *updatelist, char opt, char *opt_arg)
{
	debug_message("Entering update_set_param\n");

	error_message
	    ("Currently command line options for update are NOT supported!\n");

	debug_message("Leaving update_set_param\n");

	return FALSE;
}


/***********************************************************************
 *
 * Function:    update_row
 *
 * Summary:     Update row according to update structure
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void update_row(struct update_data *updatelist, struct row_data *row)
{
	struct update_assign_data *update_assign;


	debug_message("Entering update_row\n");

	/* Update a row completely according to update assign data */
	update_assign = updatelist_get_first_assign(updatelist);
	while (update_assign != NULL) {
		/* Update one field in current row */
		update_row_assign(update_assign, row);

		/* Next update assign */
		update_assign =
		    updatelist_get_next_assign(updatelist, update_assign);
	}			/* while */

	/* Statistics */
	updatelist->records_updated++;

	debug_message("Leaving update_row\n");
}


/***********************************************************************
 *
 * Function:    update_show_statistics
 *
 * Summary:     Show statistics
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void update_show_statistics(struct update_data *updatelist)
{
	struct update_assign_data *update_assign;


	debug_message("Entering update_show_statistics\n");

	info_message("Updating:\n");
	info_message("Records updated: %ld\n",
		     updatelist->records_updated);

	/* Statistics for all update assigns */
	update_assign = updatelist_get_first_assign(updatelist);
	while (update_assign != NULL) {
		/* Show statistics for this update assign */
		info_message("Assignment <%s> was executed %ld times\n",
			     update_assign->assign.name,
			     update_assign->num_updated);

		/* Next update assign */
		update_assign =
		    updatelist_get_next_assign(updatelist, update_assign);
	}			/* while */

	debug_message("Leaving update_show_statistics\n");
}


/***********************************************************************
 *
 * Function:    updatelist_assign_add
 *
 * Summary:     Append one update_assign to updatelist
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void
updatelist_assign_add(struct update_data *updatelist,
		      struct update_assign_data *add_update_assign)
{

	/* Add to list */
	if (add_update_assign == NULL) {
		/* No update_assign provided => nothing to add */
	} else if (updatelist == NULL) {
		/* No updatelist provided => nothing to add */
	} else if (updatelist->assign_last == NULL) {
		/* First entry into list */
		add_update_assign->next = NULL;
		updatelist->assign_last = add_update_assign;
		updatelist->assign_top = add_update_assign;
	} else {
		/* other entries exist already */
		add_update_assign->next = NULL;
		updatelist->assign_last->next = add_update_assign;
		updatelist->assign_last = add_update_assign;
	}
}

/***********************************************************************
 *
 * Function:    updatelist_assign_del
 *
 * Summary:     Remove one update_assign from updatelist
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void
updatelist_assign_del(struct update_data *updatelist,
		      struct update_assign_data *del_update_assign)
{

	/* Remove from list */
	if (del_update_assign == NULL) {
		/* No update_assign provided => nothing to remove */
	} else if (updatelist == NULL) {
		/* No updatelist provided => nothing to remove */
	} else if (updatelist->assign_last == NULL) {
		/* Nothing in list => nothing to remove */
	} else {
		if (updatelist->assign_last == del_update_assign) {
			/* Last in list => iterate through list to find new last */
			if (updatelist->assign_top != del_update_assign)
				debug_message
				    ("Removing last - inefficient use of updatelist_del\n");
			updatelist->assign_last = updatelist->assign_top;
			while (updatelist->assign_last->next != NULL
			       && updatelist->assign_last->next !=
			       del_update_assign) {
				updatelist->assign_last =
				    updatelist->assign_last->next;
			}	/* while */
		}
		if (updatelist->assign_top == del_update_assign) {
			/* First in list => remove */
			updatelist->assign_top = del_update_assign->next;
		}
	}

	/* Remove linkage */
	del_update_assign->next = NULL;
}

/***********************************************************************
 *
 * Function:    updatelist_get_first_assign
 *
 * Summary:     Get first update_assign from updatelist
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
struct update_assign_data *updatelist_get_first_assign(struct update_data
						       *updatelist)
{
	if (updatelist == NULL)
		return NULL;
	else
		return updatelist->assign_top;
}

/***********************************************************************
 *
 * Function:    updatelist_get_next_assign
 *
 * Summary:     Get next update_assign from updatelist
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
struct update_assign_data *updatelist_get_next_assign(struct update_data
						      *updatelist,
						      struct
						      update_assign_data
						      *cur_update_assign)
{
	if (cur_update_assign == NULL)
		return NULL;
	else
		return cur_update_assign->next;
}

/***********************************************************************
 *
 * Function:    update_new_assign
 *
 * Summary:     Alloc new update assign data 
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
struct update_assign_data *update_new_assign(void)
{
	return (struct update_assign_data *)
	    malloc(sizeof(struct update_assign_data));
}

/***********************************************************************
 *
 * Function:    update_free_assign
 *
 * Summary:     De-alloc update assign data
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void update_free_assign(struct update_assign_data *update_assign)
{
	assign_exit(&(update_assign->assign));

	free((void *) update_assign);
}


/***********************************************************************
 *
 * Function:    update_row_assign
 *
 * Summary:     Update one assign of a row 
 *
 * Parameters:  None
 *
 * Return:      Nothing
 *
 ***********************************************************************/
void
update_row_assign(struct update_assign_data *update_assign,
		  struct row_data *row)
{

	debug_message("Entering update_row_assign\n");

	/* No update assign? Then both are equal. */
	if (update_assign == NULL)
		return;

	/* Safety net */
	if (update_row == NULL)
		return;

	/* Statistics */
	update_assign->num_updated++;

	/* Do assign for current update field in this row */
	assign_row(&(update_assign->assign), row);

	debug_message("Leaving update_row_assign\n");
}
