//--------------------------------------------------------------
//
// Generate SQL key combinations for SELECT statements
//
//	usage:	len = fa_sql_generator_key (key, db, buffer_size, output, alias)
//		where	key is a pointer to any SQL script passed, which will be used as a key descriptor
//				db is a structure pointer to database definition data
//				buffer_size sets a limit on output to prevent buffer overflow
//				output is a pointer to an output buffer containing the generated SQL script
//				alias is a flag indicating if table aliases should be output
//		returns the length of any output
//
//	See fa_sql_def.h for database definition structures
//
// SQL commands generated should be ANSI standard compliant to support a wide variety of SQL database engines.
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#include <string.h>			// string functions such as strcmp

#include <fa_sql_def.h>		// format of SQL database, table and column definitions
#include <ut_error.h>		// error handling and debug functions from libgxtut


int fa_sql_generator_key(char *cpKey, struct fa_sql_db *spDb, int *iBuffMax, char *cpO, int iAlias)
  {
    struct fa_sql_column *spCol;	// pointer to sql column definitions
    struct fa_sql_table *spTab;		// pointer to sql table definitions

    int i;
    int iLen = 0;				// length of data added to output string - for returning to calling program
    char *cp = cpKey;			// start at begining of key template
    int iTabLen = 0;			// length of table name
    char *cpColStart = 0;		// used to hold start of column name
    int iColLen = 0;			// length of column name


	do
	  {
		if (*cp == '.')					// using alias marker to find end of table & start of column
		  {
			cpColStart=cp+1;
			iColLen=0;					// new column name

			iTabLen=1;
			while (--cp > cpKey && *(cp-1) != ' ') iTabLen++;

			i=0;
			spTab=spDb->spTab;			// start pointing to 1st table in db
			while (strncmp(spTab->sAlias, cp, iTabLen) != 0)	// Find the table using this alias
			  {
				spTab++;
				ut_check((++i < spDb->iTab), "alias not found %s len:%d", cpKey, iTabLen);
			  }
			ut_debug("key:%s table:%s",cpKey, spTab->sName);
			cp=cpColStart-1;					// return to where we were in the template string
		  }
		else if (*cp == ' ' && cpColStart > 0)	// end of column name?
		  {
			i=0;
			spCol=spTab->spCol;					// start pointing to 1st column in this table
			while (strncmp(spCol->sName, cpColStart, iColLen-1) != 0)	// Find the table using this alias
			  {
				spCol++;
				ut_check((++i < spTab->iCol),
				"column not found %s pos:%ld len:%d", cpKey, cpColStart-cpKey, iColLen-1);
			  }
			ut_debug("column:%s",spCol->sName);
			cpColStart = 0;						//ready for next column
		  }
		else if (*cp == '%')					// we should always have a table and column by now
		  {
			if (spCol->bmFlag & FA_COL_INT_B0)
			  i=snprintf(cpO, *iBuffMax, "%d", *(int *)spCol->cpPos);
			else if (spCol->bmFlag & FA_COL_CHAR_B0)
			  i=snprintf(cpO, *iBuffMax, "\'%c\'", *(spCol->cpPos));
			else
			  i=snprintf(cpO, *iBuffMax, "\'%s\'", spCol->cpPos);

			cpO+=i;
			*iBuffMax-=i;
			iLen+=i;
		  }

		if (cpColStart != 0) iColLen++;		// still parsing column name

		if (*cp == '%')						// don't output these template markers
			cp++;
		else if (*cp == '.' && !iAlias)		// don't output table aliases
		  {
			ut_debug("TabLen:%d", iTabLen);
			iLen-=iTabLen;
			*iBuffMax+=iTabLen;
			cpO-=iTabLen;					// Not using aliases so reverse back over it and the '.'
			cp++;
		  }
		else if (*iBuffMax > 0)
		  {
			*cpO++=*cp++;					// move key template to our SQL script
			(*iBuffMax)--;
			iLen++;
		  }
		else
			cp++;							// buffer full so avoid loop
	  }
	while (*cp != '\0');					// check after copying null terminator

error:
	return iLen;
  }
