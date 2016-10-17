//--------------------------------------------------------------
//
// Structures used to define a database, its tables and their columns.
//
// Used for translating SQL column data into generic data types used in C apps
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#ifndef __FA_SQL_DEF_INCLUDED__
#define __FA_SQL_DEF_INCLUDED__

#define	FA_FILENAME_S0		20		// Limits size of file or database names!
#define	FA_TABLE_NAME_S0	20		// Limits size of SQL table names!
#define	FA_ALIAS_NAME_S0	10		// Limits size of aliases used in SQL scripts!
#define	FA_COLUMN_NAME_S0	10		// Limits size of SQL column names!
#define	FA_KEY_M0			10		// Limits number of SQL key descriptors!
#define	FA_KEY_S0			100		// Limits size of SQL key descriptors! i.e. WHERE id = 0

					//---------Column flags----------
#define	FA_COL_INT_B0		0x00000001	// Identifies integer columns
#define	FA_COL_BLOB_B0		0x00000002	// Identifies blob/string columns
#define	FA_COL_CHAR_B0		0x00000004	// Identifies char/byte columns
#define	FA_COL_PRIME_B0		0x00001000	// Identifies if the column is a primary key
#define	FA_COL_AUTO_B0		0x00100000	// Identifies if the column is auto generated - ie don't INSERT it

#define	FA_FIELD_CHAR_S0	1		// Size of a char/byte field.
#define	FA_FIELD_INT_S0		4		// Size of an integer field.
					//#TODO use sizeof instead of declaring variable type sizes

#define	FA_BUFFER_S0	500			// Max size of buffers to hold SQL scripts

#define	FA_ALL_COLS_B0	0xFFFFFFFF	// Bitmap to indicate all columns are required from a SELECT

					// Definitions for each database
struct fa_sql_db
  {
    char	cName[FA_FILENAME_S0];			// null terminated file or database name (not full path)
    int		iTab;							// Number of tables in this database
    int		iColMax;						// Max number of columns found in any table in this database
    int		iKeyMax;						// Number of common keys defined for the SQL generator
    struct 	fa_sql_table *spTab;			// pointer to start of sql_table array
    char	cKey[FA_KEY_M0][FA_KEY_S0];		// null terminated SQL key string
  };

					// Definitions for each database table
struct fa_sql_table
  {
    char	cName[FA_TABLE_NAME_S0];	// null terminated SQL table name
    char	cAlias[FA_ALIAS_NAME_S0];	// null terminated SQL table alias name
    int		iCol;						// Column count per table
    struct	fa_sql_column *spCol;		// pointer to start of sql_column array
  };

					// Definitions for each database table column
struct fa_sql_column
  {
    char	cName[FA_COLUMN_NAME_S0];	// null terminated SQL column name
    int		iFlag;						// bitmap of column options
    char	*cPos;						// Where to unpack data. Note: pointers to char are guaranteed to be
										//	convertable to other pointer types - so we can cast these to
										//	 (int *) when necessary
    int		iSize;						// Size of data to unpack - max column size
  };

int fa_handler(const int, int*, struct fa_sql_db*, char*);				// for use in applications to pass file actions to libgxtfa
int fa_sql_generator(const int, int*, struct fa_sql_db*, char*, char*);	// for building SQL scripts
int fa_sql_generator_key(char*, struct fa_sql_db*, int*, char*, int);	// for building SQL SELECT key scripts
int fa_sql_handler(const int, int, char*, struct fa_sql_db*);			// for passing SQL scripts to the SQL engine

#endif
