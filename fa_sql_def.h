//--------------------------------------------------------------
//
// Used for translating SQL column data into generic data types used in C apps
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2016 [www.benningtons.net]
//
//--------------------------------------------------------------

#ifndef __FA_SQL_DEF_INCLUDED__
#define __FA_SQL_DEF_INCLUDED__

#define	SQL_DATABASE_NAME_S0	20	// Limits size of SQL database names!
#define	SQL_TABLE_NAME_S0	20		// Limits size of SQL table names!
#define	SQL_ALIAS_NAME_S0	10		// Limits size of aliases used in SQL scripts!
#define	SQL_COLUMN_NAME_S0	10		// Limits size of SQL column names!
#define	SQL_KEY_M0		10			// Limits number of SQL key descriptors
#define	SQL_KEY_S0		100			// Limits size of SQL key descriptors! i.e. WHERE id = 0

					//---------Column flags----------
#define	SQL_COL_INT_B0		0x00000001	// Identifies integer columns
#define	SQL_COL_BLOB_B0		0x00000002	// Identifies blob/string columns
#define	SQL_COL_CHAR_B0		0x00000004	// Identifies char/byte columns
#define	SQL_COL_PRIME_B0	0x00001000	// Identifies if the column is a primary key
#define	SQL_COL_AUTO_B0		0x00100000	// Identifies if the column is auto generated - ie don't INSERT it

#define	SQL_FIELD_CHAR_S0	1		// Size of a char/byte field.
#define	SQL_FIELD_INT_S0	4		// Size of an integer field.
					//#TODO use sizeof instead of declaring variable type sizes

#define	SQL_BUFFER_S0	500			// Max size of buffers to hold SQL scripts

#define	SQL_ALL_COLS_B0	0xFFFFFFFF	// Bitmap to indicate all columns are required from a SELECT

struct sql_db
  {
    char	cName[SQL_DATABASE_NAME_S0];	// null terminated SQL database name (not full path)
    int		iTab;							// Number of tables in this database
    int		iColMax;						// Max number of columns found in any table in this database
    int		iKeyMax;						// Number of common keys defined for the SQL generator
    struct sql_table *spTab;				// pointer to start of sql_table array
    char	cKey[SQL_KEY_M0][SQL_KEY_S0];	// null terminated SQL key string
  };

struct sql_table
  {
    char	cName[SQL_TABLE_NAME_S0];	// null terminated SQL table name
    char	cAlias[SQL_ALIAS_NAME_S0];	// null terminated SQL table alias name
    int		iCol;						// Column count per table
    struct sql_column *spCol;			// pointer to start of sql_column array
  };

struct sql_column
  {
    char	cName[SQL_COLUMN_NAME_S0];	// null terminated SQL column name
    int		iFlag;						// bitmap of column options
    char	*cPos;						// Where to unpack data. Note: pointers to char are guaranteed to be
										//	convertable to other pointer types - so we can cast these to
										//	 (int *) when necessary
    int		iSize;						// Size of data to unpack - max column size
  };

int fa_sql_generator(const int, int*, struct sql_db*, char*, char*);	// for building SQL scripts
int fa_sql_generator_key(char*, struct sql_db*, int*, char*, int);		// for building SQL SELECT key scripts
int fa_sql_handler(const int, int, char*, struct sql_db*);				// for passing SQL scripts to the SQL engine

#endif
