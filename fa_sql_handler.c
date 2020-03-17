//--------------------------------------------------------------
//
// Generic database handler  - to keep SQL processing code together and therefore allow easier switching
//				between database types
//
//	usage:	status = fa_sql_handler(action, SQL, database-definition)
//		where:-	action is a bitmap of filehandler commands - see fa_def.h
//				SQL points to a string containing an SQL statement or database name
//				database-definition points to a structure where the database, tables and fields are defined.
//
//		actions supported:-
//			FA_OPEN		- Open Database
//			FA_PREPARE	- Prepare (compile) an SQL command ready for stepping through results
//			FA_STEP		- Transfer SQL data by unpacking each column to a format requested by the filehandler
//			FA_RESET	- Reset a PREPARE back to it's start, ready to STEP through again
//			FA_FINALISE	- Tidily close a PREPARE-STEP-FINALISE loop - other commands will also trigger this
//			FA_EXEC		- Run an SQL command as a one-off. i.e. PREPARE-STEP-FINALISE in one go
//			FA_CLOSE	- Close Database
//
//	Keeps an index of database and command handles in fa_sql_lun.h
//
// Currently SQL commands are based on SQLITE3 but it should be possible to add compiler flags to support other SQL databases.
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2015 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <sqlite3.h>		//used for database application interface calls
#include <stdio.h>			//standard I/O
#include <string.h>			//string functions such as strcmp


#include <fa_def.h>			//filehandler actions
#include <fa_lun.h>			//table of file/database and prepared command handles
#include <fa_sql_def.h>		//format for holding details of any SQL database to enable unpacking of data
#include <ut_error.h>		//error and debug functions


int fa_sql_handler(	const int iAction,
					char *cSQL,
					struct fa_sql_db *spDB)
  {
	struct fa_sql_column *spSQLcol;		// used to step through the passed list of columns
	struct fa_sql_table *spSQLtable;	// used to step through the passed list of tables

	char sTabName[FA_TABLE_NAME_S0];	// local store for table name so we don't have to keep asking for it.
	char sColName[FA_COLUMN_NAME_S0];	// local store for column name so we don't have to keep asking for it.
	int j, i = 0;
	int ios = SQLITE_OK;				// SQLITE_OK = 0
	int iCols;							// Number of columns in a row



	if (iAction & FA_OPEN)				// open database
	  {
		ios=sqlite3_open(	cSQL,						// database filename
							&fa_lun[spDB->iLun].db);	// handle for database - used by other commands
		ut_check(ios == SQLITE_OK, "open: %d", ios);
	  }

	else if (iAction & FA_FINALISE ||				// Close down a PREPAREd statement (else memory leak)
		  (!(iAction & (FA_STEP+FA_RESET)) &&		// unless stepping or resetting
			fa_lun[spDB->iLun].row != 0))
	  												// Due to this tidy-up calling apps don't need to finalise
	  {
		ut_debug("fa_finalise");
		ios=sqlite3_finalize(fa_lun[spDB->iLun].row);		// statement handle (from FA_PREPARE) to finalise
		ut_check(ios == SQLITE_OK, "finalise");
		fa_lun[spDB->iLun].row=0;
	  }

	if (iAction & FA_STEP)							// Step through rows from a previously prepared SELECT
	  {
		ut_debug("fa_step");
		if ((ios=sqlite3_step(fa_lun[spDB->iLun].row)) == SQLITE_ROW)		// Row of data to process
		  {
			iCols=sqlite3_column_count(fa_lun[spDB->iLun].row);		// how many columns in this row?
			ut_debug("cols: %d", iCols);

			i=0;
			while (i < iCols)						// Step through each column in his row
			  {
				if (iAction & FA_COUNT)				// counts don't return original table/column names
				  {
					spSQLtable=spDB->spTab;			// So counter meta fields are added to the list of columns in the 1st table

					snprintf(sColName,				// Get column name from the counter alias used
							FA_COLUMN_NAME_S0,
							sqlite3_column_name(fa_lun[spDB->iLun].row, i));
				  }
				else
				  {
					snprintf(	sTabName,			// What table is this column from?
								FA_TABLE_NAME_S0,
								sqlite3_column_table_name(fa_lun[spDB->iLun].row, i));

					j=1;
					spSQLtable=spDB->spTab;			// look for table name in the passed list of tables
					while (strcmp(spSQLtable->sName, sTabName) != 0)
					  {
						spSQLtable++;
						ut_check (++j <= spDB->iTab, "table name not found:%s", sTabName);
//						if (++j > spDB->iTab)
//							ut_error("table name not found:%s", sTabName);
					  }

					snprintf(	sColName,			// Found table so now find the column's name
								FA_COLUMN_NAME_S0,
								sqlite3_column_origin_name(fa_lun[spDB->iLun].row, i));
				  }
				ut_debug("col name: %s", sColName);

				spSQLcol=spSQLtable->spCol;			// look for column name in this table's list
				j=1;
				while (strcmp(spSQLcol->sName, sColName) != 0)
				  {
					spSQLcol++;
					ut_check(++j <= spDB->iColMax, "column name not found:%s", sColName);
//					if (++j > spDB->iColMax) ut_error("column name not found:%s", sColName);
				  }

				ut_debug(	"matched with: %s type:%d",	spSQLcol->sName,
							spSQLcol->bmFlag);

				if (spSQLcol->bmFlag & FA_COL_INT_B0)			// unpack an integer column?
					*(int *)spSQLcol->cpPos=
						sqlite3_column_int(fa_lun[spDB->iLun].row, i);

				else if (spSQLcol->bmFlag & FA_COL_CHAR_B0)		// unpack a char/byte column?
					memcpy(	spSQLcol->cpPos,
							(char *) sqlite3_column_blob(fa_lun[spDB->iLun].row, i),
							FA_FIELD_CHAR_S0);					// copy char with no trailing null

				else											// or a string/blob column?
				  {
					char *cp = (char *) sqlite3_column_blob(fa_lun[spDB->iLun].row, i);
					if (cp == 0)								// extracting a NULL string?
						*(int *)spSQLcol->cpPos=0;
					else
						snprintf(spSQLcol->cpPos,				//output column data to field data string
								 spSQLcol->iSize,				//limit size to max column size
								 "%s",							//null terninated string data
								 cp);							//the column data
//								 (char *) sqlite3_column_blob(fa_lun[spDB->iLun].row, i));	//the column data
				  }
				i++;											// next column
			  }
			ios=FA_OK_IV0;										// return a 0 if read a row ok
		  }
		else
		  {
			ut_check(ios == SQLITE_DONE, "step %d", ios);		// if not done then bomb out to error:
			ios=FA_NODATA_IV0;									// Using a common - no record found error
		  }
	  }

	else if (iAction & FA_PREPARE)					// Prepare a custom statement ready for FA_STEP'ing
	  {
		ut_debug("fa_prepare: %s", cSQL);
		ios=sqlite3_prepare_v2(	fa_lun[spDB->iLun].db,		// database handle
								cSQL,				// SQL statement to prepare (compile)
								-1,					// Length of SQL command or up to 1st null if -1
								&fa_lun[spDB->iLun].row,	// handle for prepared statement
								0);					// pointer to unused statement (after null) if not null
		ut_check(ios == SQLITE_OK, "prepare: %d", ios);
	  }

	else if (iAction & FA_RESET)					// Reset a FA_PREPARE back to the start, ready for more FA_STEP'ing
	  {
		ios=sqlite3_reset(fa_lun[spDB->iLun].row);	// handle for prepared statement
		ut_check(ios == SQLITE_OK, "reset: %d", ios);
	  }

    else if (iAction & FA_EXEC)					// Execute a custom SQL statement as a one-off
	  {											//		with no callback routine
		ut_debug("fa_exec: %s", cSQL);
		ios=sqlite3_exec(	fa_lun[spDB->iLun].db,		// database handle
							cSQL,				// SQL command to prepare-step-finalise
							0,					// callback function - if not null
							0,					// 1st argument for callback function
							0);					// null terminated error message string or 0 if ok
		ut_check(ios == SQLITE_OK, "exec: %d", ios);
	  }

    else if (iAction & FA_CLOSE)				// Close database
	  {
	    if (spDB->iLun > 0)						// check db is open
			if (fa_lun[spDB->iLun].db > 0)
				sqlite3_close(fa_lun[spDB->iLun].db);
	  }
	else if (!(iAction & (FA_FINALISE+FA_OPEN)))	// Ignore as already dealt with above
	  {
		ut_error("unknown: %x", iAction);
		ios=-1;										// Unknown command passed?
	  }

	return ios;

error:
	ut_error("%s", sqlite3_errmsg(fa_lun[spDB->iLun].db));			// A more informative error description
	return ios;
  }
