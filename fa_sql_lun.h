//--------------------------------------------------------------
//
// An index of database handles. Enabling processes to manage many open databases
//	See fa_def,h for index parameters
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2015 [www.benningtons.net]
//
//--------------------------------------------------------------

#include	<sqlite3.h>

#define	FL_M0	150

struct
  {
    sqlite3 *db[FL_M0];
    sqlite3_stmt *row[FL_M0];
  } sql_lun;

