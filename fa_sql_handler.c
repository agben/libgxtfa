//--------------------------------------------------------------
//
// Generic database handler  - to keep SQL processing code together and therefore allow easier switching
//				between database types
//
//	usage:	status = fa_sql_handler(action, db_lun, SQL, database-definition)
//		where:-	action is a bitmap of filehandler commands - see fa_def.h
//				db_lun is an index to a table of database and command handles
//				SQL points to a string containing an SQL statement or database name
//				database-definition points to where the database, tables and fields are defined.
//
//		actions supported:-
//			FA_OPEN		- Open Database
//			FA_PREPARE	- Prepare (compile) an SQL command ready for stepping through results
//			FA_STEP		- Transfer SQL data by unpacking each column to a format requested by the filehandler
//			FA_FINALISE	- Tidily close a PREPARE-STEP-FINALISE loop - other commands will also trigger this
//			FA_EXEC		- Run an SQL command as a one-off. i.e. PREPARE-STEP-FINALISE in one go
//			FA_CLOSE	- Close Database
//
//	Keeps an index of database and command handles in sql_lun.h which should only be used by this routine.
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
#include <fa_sql_def.h>		//format for holding details of any SQL database to enable unpacking of data
#include <fa_sql_lun.h>		//table of database and prepared command handles
#include <ut_error.h>		//error and debug functions


int fa_sql_handler(	const int iAction,
					int iDb,
					char *cSQL,
					struct sql_db *spDb)
  {
	char cBuff[50];
	struct sql_column *spSQLcol;	// used to step through the passed list of columns
	struct sql_table *spSQLtable;	// used to step through the passed list of tables
	char cTabName[SQL_TABLE_NAME_S0];	// local store for table name so we don't have to keep asking for it.
	char cColName[SQL_COLUMN_NAME_S0];	// local store for column name so we don't have to keep asking for it.
	int j, i = 0;
	int ios = SQLITE_OK;			// SQLITE_OK = 0
	int iCols;						// Number of columns in a row



    if (sql_lun.db[iDb] == 0 ||		// Auto Open database on 1st access
			(iAction & FA_OPEN))	// Or requested open database
	  {
		ut_debug("open");
		snprintf(cBuff, 50, "/var/local/%s", spDb->cName);		//#TODO - set size and full path properly

		ios=sqlite3_open(	cBuff,			// database filename
				&sql_lun.db[iDb]);			// handle for database - used by other commands
		ut_check(ios == SQLITE_OK, "open: %d", ios);
	  }

	if (iAction & FA_FINALISE ||				// Close down a PREPAREd statement (else memory leak)
		(!(iAction & FA_STEP) && sql_lun.row[iDb] != 0))		// Don't close down a PREPAREd statement if stepping
	  {											// Cos of this tidy-up apps don't need to finalise
		ut_debug("fa_finalise");
		ios=sqlite3_finalize(sql_lun.row[iDb]);		// statement handle (from FA_PREPARE) to finalise
		ut_check(ios == SQLITE_OK, "finalise");
		sql_lun.row[iDb]=0;
	  }

	if (iAction & FA_STEP)							// Step through rows from a previously prepared SELECT
	  {
		ut_debug("fa_step");
		if ((ios=sqlite3_step(sql_lun.row[iDb])) == SQLITE_ROW)		// Row of data to process
		  {
			iCols=sqlite3_column_count(sql_lun.row[iDb]);		// how many columns in this row?
			ut_debug("cols: %d", iCols);

			i=0;
			while (i < iCols)						// Step through each column in his row
			  {
				snprintf(	cTabName,				// What table is this column from?
							SQL_TABLE_NAME_S0,
							sqlite3_column_table_name(sql_lun.row[iDb], i));

				j=1;
				spSQLtable=spDb->spTab;				// look for table name in the passed list of tables
				while (strcmp(spSQLtable->cName, cTabName) != 0)
				  {
					spSQLtable++;
					if (++j > spDb->iTab)
					ut_error("table name not found:%s", cTabName);
				  }

				snprintf(	cColName,				// Found table so now find the column's name
							SQL_COLUMN_NAME_S0,
							sqlite3_column_origin_name(sql_lun.row[iDb], i));
				ut_debug("col name: %s", cColName);

				spSQLcol=spSQLtable->spCol;			// look for column name in this table's list
				j=1;
				while (strcmp(spSQLcol->cName, cColName) != 0)
				  {
					spSQLcol++;
					if (++j > spDb->iColMax) ut_error("column name not found:%s", cColName);
				  }

				ut_debug(	"matched with: %s type:%d",	spSQLcol->cName,
							spSQLcol->iFlag);

				if (spSQLcol->iFlag & SQL_COL_INT_B0)			// unpack an integer column?
					*(int *)spSQLcol->cPos=
						sqlite3_column_int(sql_lun.row[iDb], i);
				else if (spSQLcol->iFlag & SQL_COL_CHAR_B0)		// unpack a char/byte column?
					snprintf(	spSQLcol->cPos,					//output column data to field data string
								spSQLcol->iSize,				//limit size to max column size
								"%c",							//unterninated string data
								sqlite3_column_bytes(sql_lun.row[iDb], i));	//the column data
				else											// or a string/blob column?
					snprintf(	spSQLcol->cPos,					//output column data to field data string
								spSQLcol->iSize,				//limit size to max column size
								"%s",							//null terninated string data
								(char *) sqlite3_column_blob(sql_lun.row[iDb], i));	//the column data
					i++;										// next column
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
		ios=sqlite3_prepare_v2(	sql_lun.db[iDb],	// database handle
								cSQL,				// SQL statement to prepare (compile)
								-1,					// Length of SQL command or up to 1st null if -1
								&sql_lun.row[iDb],	// handle for prepared statement
								0);					// pointer to unused statement (after null) if not null
		ut_check(ios == SQLITE_OK, "prepare: %d", ios);
	  }

    else if (iAction & FA_EXEC)					// Execute a custom SQL statement as a one-off
	  {											//		with no callback routine
		ut_debug("fa_exec: %s", cSQL);
		ios=sqlite3_exec(	sql_lun.db[iDb],	// database handle
							cSQL,				// SQL command to prepare-step-finalise
							0,					// callback function - if not null
							0,					// 1st argument for callback function
							0);					// null terminated error message string or 0 if ok
		ut_check(ios == SQLITE_OK, "exec: %d", ios);
	  }

    else if (iAction & FA_CLOSE)				// Close database
		sqlite3_close(sql_lun.db[iDb]);

    else if (!(iAction & (FA_FINALISE+FA_OPEN)))	// Ignore as already dealt with above
      {
		ut_error("unknown: %d", iAction);
		ios=-1;									// Unknown command passed?
	  }

	return ios;


error:
	ut_error("%s", sqlite3_errmsg(sql_lun.db[iDb]));			// A more informative error description
	return ios;
  }
