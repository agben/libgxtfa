//--------------------------------------------------------------
//
// Generic file/Database handler
//
//	usage:	status = fa_handler (action, fields, DB, SQL)
//		where	action is a bitmap of filehandler commands - see fa_def.h
//				fields points to a bitmap of fields to read/update
//				DB points to a file/database definitions structure
//				SQL is an optional SQL script to pass onto certain actions
//
//	Available actions:
//		FA_OPEN		- Open Database
//		FA_CLOSE	- Close Database
//		FA_READ		- Prepare a SELECT command. Can be used with FA_STEP to return the result of the 1st STEP
//		FA_WRITE	- Prepare an INSERT command to add a row to the database
//		FA_UPDATE	- Prepare an UPDATE command to update selected fields in the database
//		FA_PREPARE	- An adhoc query so pass the SQL instruction on to the sql_handler
//		FA_STEP		- Return the next row of data from an FA_READ
//		FA_FINALISE	- Tidily close a SELECT-STEP-FINALISE loop - other commands will also trigger this
//		FA_EXEC		- Pass on a passed SQL instruction for execution in a single SELECT-STEP-FINALISE action
//		FA_DELETE	- Prepare a DELETE command to remove a row from the database
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <fa_def.h>			// file/db actions
//#include <ce_main.h>		// definition of memory structure holding interim CE data
//#include <ce_main_def.h>	// database, table and column definitions for CE
#include <ut_error.h>		// error handling and debug functions

int fa_handler(int iAction, int *ipField, struct fa_sql_db *spDB, char *cpSQL)
{
	char cBuff[FA_BUFFER_S0];		// SQL command input buffer	#TODO - use malloc and a common SQL size
	char *cp = &cBuff[0];
	int i, j;
	int ios = 0;


	ut_debug("action:%x", iAction);

	if (iAction & (FA_PREPARE+FA_EXEC))					// Use the passed SQL script for adhoc actions
		cp=cpSQL;
	else if (iAction & (FA_WRITE+FA_READ+FA_UPDATE+FA_DELETE))	// generate an SQL script from the details passed
	 {
		ut_check(fa_sql_generator(	iAction,			// Pass on the action
									ipField,			// Pointer to requested fields bitmap
									spDB,				// Database definition
									cpSQL,				// pass any SQL script fed into the filehandler
									cp) == 0,			// pointer to output buffer for generated scipt
				"SQL gen fail");						// will jump to error: if a problem
		cp=&cBuff[0];									// point back to the start ready for passing
		if (!(iAction & FA_READ)) iAction=FA_EXEC;		// SQL script is prepared so now execute it
	 }

	if (iAction & (FA_PREPARE+FA_FINALISE+FA_EXEC+FA_READ+FA_OPEN+FA_CLOSE))	// Pass these SQL commands straight through
	 {
		if (iAction & FA_READ)							// FA_READ will cause a PREPARE to be followed by a STEP
			i=FA_PREPARE;
		else
			i=iAction;

		if (i & (FA_PREPARE+FA_EXEC)) ut_debug("SQL=%s", cp);	// check on prepared SQL scripts

		ios=fa_sql_handler(	i,							// Pass on the action
@@@							CE_MAIN_L0,					// Database handle
							cp,							// SQL command
							spDB);						// Database definition
		ut_check (ios == 0,"%d", ios);					// jumps to error: if not true
	 }

	else
		ut_check(!(ios & FA_STEP), "unknown: %d", iAction);		// Ignore FA_STEP as dealt with below


	if (iAction & FA_STEP)								// Step through rows from a previously prepared SELECT
	 {
		ios=fa_sql_handler(	FA_STEP,					// Action
@@@							CE_MAIN_L0,					// Database handle
							0,							// not used
							spDB);						// Field definitions
		if (ios == FA_OK_IV0)							// Data returned
		 {												// #TODO remove following debug before publishing
/*
			for (i=0; i < CEB.iTab; i++)
			 {
				for (j=0; j < CET[i].iCol; j++)
				 {
					if ((iCEField[i]>>j) & 1)
					 {
						if (CEF[i][j].iFlag & FA_COL_INT_B0)
						 {
							ut_debug(	"unpack:%s :%d",
										CEF[i][j].cName,
										*(int *)CEF[i][j].cPos);
						 }
						else
						 {
							ut_debug(	"unpack:%s :%s",
										CEF[i][j].cName,
										CEF[i][j].cPos);
						 }
					 }
				 }
			 }
*/
		 }
		else
			ut_check(	ios == FA_NODATA_IV0,
						"step error %d", ios);			// Ignore no data found or end of row messages
	 }

error:
	return ios;
}
