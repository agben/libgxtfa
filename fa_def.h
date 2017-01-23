//--------------------------------------------------------------
//
// Common file and database access commands to enable storage and filesystem agnostic code
// These parameters are bitmaps that may be combined in various combinations to achieve a wide
//	variety of commands for filehandlers to interpret.
//
//	GNU GPLv3 licence	libgxtfa by Andrew Bennington 2015 [www.benningtons.net]
//
//--------------------------------------------------------------

#define	FA_KEY0		0x00000000		// Key ID's - used in combinations to achieve many permutations
#define	FA_KEY1		0x00000001
#define	FA_KEY2		0x00000002		// key 0 assumed to be the default - usually id on db's
#define	FA_KEY3		0x00000003
#define	FA_KEY4		0x00000004
#define	FA_KEY5		0x00000005
#define	FA_KEY6		0x00000006
//	reserved	0x00000008
//	reserved	0x00000010
//	reserved	0x00000020
//	reserved	0x00000040
//	reserved	0x00000080
#define	FA_KEY_MASK	0x000000FF		// mask other actions and use key bits as a number not a bitmap.

#define	FA_OPEN		0x00000100		// Common file actions
#define	FA_CLOSE	0x00000200
#define	FA_READ		0x00000400
#define	FA_WRITE	0x00000800
#define	FA_UPDATE	0x00001000
#define	FA_DELETE	0x00002000
//	spare		0x00004000
//	spare		0x00008000

#define	FA_PREPARE	0x00010000		// Common SQL actions
#define	FA_STEP		0x00020000
#define	FA_FINALISE	0x00040000
#define	FA_EXEC		0x00080000
#define	FA_INIT		0x00100000
#define	FA_DISTINCT	0x00200000
//	spare		0x00400000
//	spare		0x00800000

#define	FA_LINK		0x01000000		// Filehandler defined actions
#define	FA_PURGE	0x02000000
//	spare		0x04000000
//	spare		0x08000000
//	spare		0x10000000
//	spare		0x20000000
//	spare		0x40000000
//	spare		0x80000000

// Common error codes to allow storage agnostic error handling
#define	FA_OK_IV0	0			// Universal all ok code
#define	FA_NODATA_IV0	36		// Using the old openVMS definitions...

// Column/field selections
#define	FA_ALL_COLS_B0	0xFFFFFFFF	// Bitmap to indicate all columns are required from a SELECT
