//--------------------------------------------------------------
//
// Generic file/Database handler functions
//
//	usage:	status = fa_handler (action, DB, SQL)
//		where action is a bitmap of filehandler commands - see fa_def.h
//				DB points to a database definitions structure
//			and SQL is an optional SQL script to pass onto certain actions
//
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

#include <stdio.h>			// standard I/O
#include <string.h>			// string functions such as strncmp

#include <fa_def.h>			// file/db actions
#include <fa_sql_def.h>		// format for holding details of any SQL database to enable unpacking of data
#include <fa_sql_lun.h>		// table of database and prepared command handles
#include <ut_error.h>		// error handling and debug functions


int fa_handler(int iAction, struct fa_sql_db *spDB, char *cpSQL)
{
	char szBuff[FA_BUFFER_S0];		// SQL command input buffer	#TODO - use malloc and a common SQL size
	int i;
	int ios = 0;
	char *cp = &szBuff[0];


	ut_debug("action:%x", iAction);

	if (iAction & (FA_PREPARE+FA_EXEC))					// Use the passed SQL script for adhoc actions
		cp=cpSQL;
	else if (iAction & (FA_WRITE+FA_READ+FA_UPDATE+FA_DELETE))	// generate an SQL script from the details passed
	 {
		ut_check(fa_sql_generator(	iAction,			// Pass on the action
									spDB,				// Database definition
									cpSQL,				// pass any SQL script fed into the filehandler
									cp) == 0,			// pointer to output buffer for generated scipt
				"SQL gen fail");						// will jump to error: if a problem
		cp=&szBuff[0];									// point back to the start ready for passing
		if (!(iAction & FA_READ)) iAction=FA_EXEC;		// SQL script is prepared so now execute it
	 }
	else if (iAction & FA_INIT)							//intitalise libgxtfa when starting a process
		for (i=0; i < FA_LUN_M0; fa_lun[i++].szFile[0]=0);

	if (iAction & (FA_PREPARE+FA_FINALISE+FA_EXEC+FA_READ+FA_OPEN+FA_CLOSE))	// Pass these SQL commands straight through
	 {
		if (iAction & FA_READ)							// FA_READ will cause a PREPARE followed by a STEP
			i=FA_PREPARE;
		else if (iAction & FA_OPEN)						// Allocate a lun slot for db and transaction handles
		 {
			spDB->iLun=-1;								// mark lun as being allocated

			snprintf(	szBuff,
						FA_FULLNAME_S0,
						"%s%s",
						spDB->szPath,					// path name
						spDB->szFile);					// file name

			i=0;
			while (i < FA_LUN_M0)
			 {
				if (strncmp(fa_lun[i].szFile, szBuff, FA_FULLNAME_S0) == 0)
				  {
					spDB->iLun=i;				// file already open so re-instate lun
					i=0;						// mark as no error (ok to continue)
					ut_error("file already open %s", szBuff);	// but flag the issue anyhow
				  }
				else if (spDB->iLun < 0 && fa_lun[i].szFile[0] == 0)
					spDB->iLun=i;				// remember the 1st empty lun slot
				i++;
			 }

			ut_check(	spDB->iLun >= 0,		// Room for another open file?
						"lun slots full");
		 }
		else
			i=iAction;

		if (i & (FA_PREPARE+FA_EXEC)) ut_debug("SQL=%s", cp);	// check on prepared SQL scripts

		ios=fa_sql_handler(	i,						// Pass on the action
							cp,						// SQL command
							spDB);					// Database definition
		ut_check (ios == 0,"%d", ios);				// jumps to error: if not true

		if (iAction & FA_CLOSE)						// Closed file/db so release lun
		 {
			fa_lun[spDB->iLun].szFile[0]=0;			// free lun slot for re-use
			spDB->iLun=0;							// clear lun in db definitions
		 }
		else if (iAction & FA_OPEN)
		 {
			snprintf(	fa_lun[spDB->iLun].szFile,
						FA_FULLNAME_S0,
						"%s",
						szBuff);					// Mark lun slot as used for this db
		 }
	 }
	else
		ut_check(!(ios & (FA_STEP+FA_INIT)), "unknown: %d", iAction);		// Valid action passed?


	if (iAction & FA_STEP)							// Step through rows from a previously prepared SELECT
	 {
		ios=fa_sql_handler(	FA_STEP,				// Action
							0,						// not used
							spDB);					// Field definitions
		if (ios != FA_OK_IV0)
			ut_check(	ios == FA_NODATA_IV0,
						"step error %d", ios);		// Ignore no data found or end of row messages
	 }

error:
	return ios;
}
