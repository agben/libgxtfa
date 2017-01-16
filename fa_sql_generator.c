//--------------------------------------------------------------
//
// Generate SQL scripts based on a bitmap of 'actions' and a bitmap of tables/columns to work with
//
//	usage:	status = fa_sql_generator (action, db, key, output)
//		where	action is a bitmap of filehandler commands - see fa_def.h
//				db is a structure pointer to database definition data
//				key is a pointer to any SQL script passed, which will be used as a key descriptor
//				output is a pointer to an output buffer containing the generated SQL script
//					the max buffer size is set by FA_BUFFER_S0 in fa_sql_def.h
//		returns 0 if ok, else -1
//
//	See fa_sql_def.h for database definition structures
//
// SQL commands generated should be ANSI standard compliant to support a wide variety of SQL database engines.
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <stdio.h>

#include <fa_def.h>			// file/db actions
#include <fa_sql_def.h>		// format of SQL database, table and column definitions
#include <ut_error.h>		// error handling and debug functions from libgxtut

#define TRUE	1
#define FALSE	0


int fa_sql_generator(int iAction, struct fa_sql_db *spDb, char *cpPKey, char *cpO)
  {
    struct fa_sql_column *spCol;					// pointer to sql column definitions
    struct fa_sql_table *spTab;						// pointer to sql table definitions

    int iBuffMax = FA_BUFFER_S0;					// max buffer size
    int i, j;
    char *cpKey = &spDb->sKey[iAction & FA_KEY_MASK][0];		// pointer to sql key definitions


    spTab=spDb->spTab;								// start pointing to 1st table in db
    i=0;
    while (spTab->bmField == 0)						// any fields requested from this table?
	  {
		spTab++;									// no, so next table
		ut_check((++i < spDb->iTab),"no fields");	// will jump to error: if exceeded db's table count
      }

//    ut_debug("table:%d, fields:%x", i, *ipField);

	if (iAction & FA_READ)							// Prepare SQL to SELECT from db
	  {
		j=snprintf(cpO, iBuffMax, "SELECT ");
		cpO+=j;										// step through the output buffer
		iBuffMax-=j;								// whilst reducing the remaining buffer space

		if (spTab->bmField == FA_ALL_COLS_B0)
		  {
			snprintf(cpO, iBuffMax, "*");
			cpO++;
			iBuffMax--;
		  }
		else
		  {
			if (iAction & FA_DISTINCT)				// SELECT DISTINCT i.e. only return distinct (different) values
			  {
				j=snprintf(	cpO,
							iBuffMax,
							"DISTINCT ");
				cpO+=j;
				iBuffMax-=j;
			  }

			spCol=spTab->spCol;
			for (i=0; i < spTab->iCol; i++)			// List columns to SELECT
			  {
				if ((spTab->bmField>>i) & 1)
				  {
					j=snprintf(	cpO,
								iBuffMax,
								"%s.%s, ",
								spTab->sAlias,		// Table alias
								spCol->sName);		// Column name
					cpO+=j;
					iBuffMax-=j;
				  }
				spCol++;
			  }
			cpO-=2;									// reverse back over the last ", "
			iBuffMax+=2;
		  }

		j=snprintf(	cpO,
					iBuffMax,
					" FROM %s AS %s WHERE ",
					spTab->sName,
					spTab->sAlias);
		cpO+=j;
		iBuffMax-=j;

		if (cpPKey != 0) cpKey=cpPKey;				// use the passed key rather than any specified by FA_KEYx

		cpO+=fa_sql_generator_key(	cpKey,			// selected key details
									spDb,			// selected database details
									&iBuffMax,		// remaining output buffer
									cpO,			// output buffer
									TRUE);			// use table aliases on all columns

		j=snprintf(cpO, iBuffMax, ";");
	  }

	else if (iAction & FA_UPDATE)					// UPDATE a row in the database
	  {												// #TODO may be better to update changed fields only?
		j=snprintf(cpO, iBuffMax, "UPDATE %s SET ", spTab->sName);
		cpO+=j;										// step through the output buffer
		iBuffMax-=j;								// whilst reducing the remaining buffer space

		spCol=spTab->spCol;
		for (i=0; i < spTab->iCol; i++)
		  {
			if ((spTab->bmField>>i) & 1)
			  {
				j=snprintf(cpO, iBuffMax, "%s=", spCol->sName);	// output list of selected field names
				cpO+=j;
				iBuffMax-=j;
				if (spCol->bmFlag & FA_COL_INT_B0)			// integer data
					j=snprintf(cpO, iBuffMax, "%d, ", *(int *)spCol->cpPos);
				else if (spCol->bmFlag & FA_COL_CHAR_B0)		// single char/byte data
					j=snprintf(cpO, iBuffMax, "\"%c\", ", *(spCol->cpPos));
				else										// else string/blob data
					j=snprintf(cpO, iBuffMax, "\"%s\", ", spCol->cpPos);

				cpO+=j;
				iBuffMax-=j;
			  }
			spCol++;
		  }
		cpO-=2;									// reverse back over the last ", "
		iBuffMax+=2;

		j=snprintf(cpO, iBuffMax, " WHERE ");
		cpO+=j;
		iBuffMax-=j;

		if (cpPKey != 0) cpKey=cpPKey;			// use the passed key rather than any specified by FA_KEYx

		j=fa_sql_generator_key(	cpKey,			// selected key details
								spDb,			// selected database details
								&iBuffMax,		// remaining output buffer
								cpO,			// output buffer
								FALSE);			// use NO table aliases on columns
		cpO+=j;
		iBuffMax-=j;

		cpO+=snprintf(cpO, iBuffMax, ";");

	  }

	else if (iAction & FA_WRITE)				// INSERT a row into the database
      {
		j=snprintf(cpO, iBuffMax, "INSERT INTO %s (", spTab->sName);
		cpO+=j;									// step through the output buffer
		iBuffMax-=j;							// whilst reducing the remaining buffer space

		spCol=spTab->spCol;
		for (i=0; i < spTab->iCol; i++)
		  {
			if (((spTab->bmField>>i) & 1) && !(spCol->bmFlag & FA_COL_AUTO_B0))
			  {									// Don't try writing to any auto-generated columns
				j=snprintf(cpO, iBuffMax, "%s, ", spCol->sName);	// output list of selected field names
				cpO+=j;
				iBuffMax-=j;
			  }
			spCol++;
		  }
		cpO-=2;								// reverse back over the last ", "
		iBuffMax+=2;

		j=snprintf(cpO, iBuffMax, ") VALUES (");
		cpO+=j;
		iBuffMax-=j;

		spCol=spTab->spCol;
		for (i=0; i < spTab->iCol; i++)
		  {
			if (((spTab->bmField>>i) & 1) && !(spCol->bmFlag & FA_COL_AUTO_B0))
			  {								// Don't try writing to any auto-generated columns
				if (spCol->bmFlag & FA_COL_INT_B0)
					j=snprintf(cpO, iBuffMax, "%d, ", *(int *)spCol->cpPos);
				else if (spCol->bmFlag & FA_COL_CHAR_B0)
					j=snprintf(cpO, iBuffMax, "\"%c\", ", *(spCol->cpPos));
				else
					j=snprintf(cpO, iBuffMax, "\"%s\", ", spCol->cpPos);

				cpO+=j;
				iBuffMax-=j;
			  }
			spCol++;
		  }

		cpO-=2;								// reverse back over the last ", "
		iBuffMax+=2;

		cpO+=snprintf(cpO, iBuffMax, ");");
	  }

	else if (iAction & FA_DELETE)			// DELETE a row from the database
	  {
		j=snprintf(	cpO,
					iBuffMax,
					"DELETE FROM %s WHERE ", spTab->sName);
		cpO+=j;
		iBuffMax-=j;

		if (cpPKey != 0) cpKey=cpPKey;		// use the passed key rather than any specified by FA_KEYx

		j=fa_sql_generator_key(	cpKey,		// selected key details
								spDb,		// selected database details
								&iBuffMax,	// remaining output buffer
								cpO,		// output buffer
								FALSE);		// use NO table aliases on columns
		cpO+=j;
		iBuffMax-=j;

		cpO+=snprintf(cpO, iBuffMax, ";");
	  }

    else
		ut_error("unknown: %d", iAction);

    return 0;

error:
    return -1;
  }
