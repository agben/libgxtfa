//--------------------------------------------------------------
//
// An index of database handles. Enabling processes to manage many open databases
//
// These are held here, rather than in fa_sql_def.h, due to the database definitions structure being agnostic
//	about the database engine being used. Whereas these handles will vary in format for each file/database type used.
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2015 [www.benningtons.net]
//
//--------------------------------------------------------------

#include	<sqlite3.h>

#include	<fa_sql_def.h>

#define	FA_LUN_M0	50			// Sets max number of concurrently open files

struct
  {
	char sFile[FA_FULLNAME_S0];
	sqlite3 *db;
	sqlite3_stmt *row;
  } fa_lun[FA_LUN_M0];

// #TODO should prepare statements at start-up and re-use them with sqlite3_bind and reset.
//	they could then replace the use of FA_KEY% in the filehandler? Also then not necessary to strip quotes from saved text.
