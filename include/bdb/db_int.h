/* DO NOT EDIT: automatically built by dist/s_win32. */
/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996-2005
 *	Sleepycat Software.  All rights reserved.
 *
 * $Id: db_int.in,v 12.15 2005/11/03 17:46:08 bostic Exp $
 */

#ifndef _DB_INTERNAL_H_
#define	_DB_INTERNAL_H_

/*******************************************************
 * System includes, db.h, a few general DB includes.  The DB includes are
 * here because it's OK if db_int.h includes queue structure declarations.
 *******************************************************/
#ifndef NO_SYSTEM_INCLUDES
#if defined(STDC_HEADERS) || defined(__cplusplus)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <errno.h>
#endif

#include "db.h"

#include "dbinc/queue.h"
#include "dbinc/shqueue.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************
 * General purpose constants and macros.
 *******************************************************/
#ifndef	UINT16_MAX
#define	UINT16_MAX	65535		/* Maximum 16-bit unsigned. */
#endif
#ifndef	UINT32_MAX
#ifdef __STDC__
#define	UINT32_MAX	4294967295U	/* Maximum 32-bit unsigned. */
#else
#define	UINT32_MAX	0xffffffff	/* Maximum 32-bit unsigned. */
#endif
#endif

#if defined(HAVE_64BIT_TYPES)
#undef	INT64_MAX
#undef	INT64_MIN
#undef	UINT64_MAX

#ifdef	DB_WIN32
#define	INT64_MAX	_I64_MAX
#define	INT64_MIN	_I64_MIN
#define	UINT64_MAX	_UI64_MAX

#define	INT64_FMT	"%l64d"
#define	UINT64_FMT	"%l64u"
#else
/*
 * Override the system's 64-bit min/max constants.  AIX's 32-bit compiler can
 * handle 64-bit values, but the system's constants don't include the LL/ULL
 * suffix, and so can't be compiled using the 32-bit compiler.
 */
#define	INT64_MAX	9223372036854775807LL
#define	INT64_MIN	(-INT64_MAX-1)
#define	UINT64_MAX	18446744073709551615ULL



#endif	/* DB_WIN32 */
#endif	/* HAVE_LONG_LONG && HAVE_UNSIGNED_LONG_LONG */

#define	MEGABYTE	1048576
#define	GIGABYTE	1073741824

#define	MS_PER_SEC	1000		/* Milliseconds in a second. */
#define	USEC_PER_MS	1000		/* Microseconds in a millisecond. */

#define	RECNO_OOB	0		/* Illegal record number. */

/* Test for a power-of-two (tests true for zero, which doesn't matter here). */
#define	POWER_OF_TWO(x)	(((x) & ((x) - 1)) == 0)

/* Test for valid page sizes. */
#define	DB_MIN_PGSIZE	0x000200	/* Minimum page size (512). */
#define	DB_MAX_PGSIZE	0x010000	/* Maximum page size (65536). */
#define	IS_VALID_PAGESIZE(x)						\
	(POWER_OF_TWO(x) && (x) >= DB_MIN_PGSIZE && ((x) <= DB_MAX_PGSIZE))

/* Minimum number of pages cached, by default. */
#define	DB_MINPAGECACHE	16

/*
 * If we are unable to determine the underlying filesystem block size, use
 * 8K on the grounds that most OS's use less than 8K for a VM page size.
 */
#define	DB_DEF_IOSIZE	(8 * 1024)

/* Align an integer to a specific boundary. */
#undef	DB_ALIGN
#define	DB_ALIGN(v, bound)						\
	(((v) + (bound) - 1) & ~(((uintmax_t)(bound)) - 1))

/* Increment a pointer to a specific boundary. */
#undef	ALIGNP_INC
#define	ALIGNP_INC(p, bound)						\
	(void *)(((uintptr_t)(p) + (bound) - 1) & ~(((uintptr_t)(bound)) - 1))

/* Decrement a pointer to a specific boundary. */
#undef	ALIGNP_DEC
#define	ALIGNP_DEC(p, bound)						\
	(void *)((uintptr_t)(p) & ~(((uintptr_t)(bound)) - 1))

/*
 * Print an address as a u_long (a u_long is the largest type we can print
 * portably).  Most 64-bit systems have made longs 64-bits, so this should
 * work.
 */
#define	P_TO_ULONG(p)	((u_long)(uintptr_t)(p))

/*
 * Convert a pointer to a small integral value.
 *
 * The (u_int16_t)(uintptr_t) cast avoids warnings: the (uintptr_t) cast
 * converts the value to an integral type, and the (u_int16_t) cast converts
 * it to a small integral type so we don't get complaints when we assign the
 * final result to an integral type smaller than uintptr_t.
 */
#define	P_TO_UINT32(p)	((u_int32_t)(uintptr_t)(p))
#define	P_TO_UINT16(p)	((u_int16_t)(uintptr_t)(p))

/*
 * There are several on-page structures that are declared to have a number of
 * fields followed by a variable length array of items.  The structure size
 * without including the variable length array or the address of the first of
 * those elements can be found using SSZ.
 *
 * This macro can also be used to find the offset of a structure element in a
 * structure.  This is used in various places to copy structure elements from
 * unaligned memory references, e.g., pointers into a packed page.
 *
 * There are two versions because compilers object if you take the address of
 * an array.
 */
#undef	SSZ
#define	SSZ(name, field)  P_TO_UINT16(&(((name *)0)->field))

#undef	SSZA
#define	SSZA(name, field) P_TO_UINT16(&(((name *)0)->field[0]))

/* Structure used to print flag values. */
typedef struct __fn {
	u_int32_t mask;			/* Flag value. */
	const char *name;		/* Flag name. */
} FN;

/* Set, clear and test flags. */
#define	FLD_CLR(fld, f)		(fld) &= ~(f)
#define	FLD_ISSET(fld, f)	((fld) & (f))
#define	FLD_SET(fld, f)		(fld) |= (f)
#define	F_CLR(p, f)		(p)->flags &= ~(f)
#define	F_ISSET(p, f)		((p)->flags & (f))
#define	F_SET(p, f)		(p)->flags |= (f)
#define	LF_CLR(f)		((flags) &= ~(f))
#define	LF_ISSET(f)		((flags) & (f))
#define	LF_SET(f)		((flags) |= (f))

/*
 * Calculate a percentage.  The values can overflow 32-bit integer arithmetic
 * so we use floating point.
 *
 * When calculating a bytes-vs-page size percentage, we're getting the inverse
 * of the percentage in all cases, that is, we want 100 minus the percentage we
 * calculate.
 */
#define	DB_PCT(v, total)						\
	((int)((total) == 0 ? 0 : ((double)(v) * 100) / (total)))
#define	DB_PCT_PG(v, total, pgsize)					\
	((int)((total) == 0 ? 0 :					\
	    100 - ((double)(v) * 100) / (((double)total) * (pgsize))))

/*
 * Structure used for callback message aggregation.
 *
 * Display values in XXX_stat_print calls.
 */
typedef struct __db_msgbuf {
	char *buf;			/* Heap allocated buffer. */
	char *cur;			/* Current end of message. */
	size_t len;			/* Allocated length of buffer. */
} DB_MSGBUF;
#define	DB_MSGBUF_INIT(a) do {						\
	(a)->buf = (a)->cur = NULL;					\
	(a)->len = 0;							\
} while (0)
#define	DB_MSGBUF_FLUSH(dbenv, a) do {					\
	if ((a)->buf != NULL) {						\
		if ((a)->cur != (a)->buf)				\
			__db_msg(dbenv, "%s", (a)->buf);		\
		__os_free(dbenv, (a)->buf);				\
		DB_MSGBUF_INIT(a);					\
	}								\
} while (0)
#define	STAT_FMT(msg, fmt, type, v) do {				\
	DB_MSGBUF __mb;							\
	DB_MSGBUF_INIT(&__mb);						\
	__db_msgadd(dbenv, &__mb, fmt, (type)(v));			\
	__db_msgadd(dbenv, &__mb, "\t%s", msg);				\
	DB_MSGBUF_FLUSH(dbenv, &__mb);					\
} while (0)
#define	STAT_HEX(msg, v)						\
	__db_msg(dbenv, "%#lx\t%s", (u_long)(v), msg)
#define	STAT_ISSET(msg, p)						\
	__db_msg(dbenv, "%sSet\t%s", (p) == NULL ? "!" : " ", msg)
#define	STAT_LONG(msg, v)						\
	__db_msg(dbenv, "%ld\t%s", (long)(v), msg)
#define	STAT_LSN(msg, lsnp)						\
	__db_msg(dbenv, "%lu/%lu\t%s",					\
	    (u_long)(lsnp)->file, (u_long)(lsnp)->offset, msg)
#define	STAT_POINTER(msg, v)						\
	__db_msg(dbenv, "%#lx\t%s", P_TO_ULONG(v), msg)
#define	STAT_STRING(msg, p) do {					\
	const char *__p = p;	/* p may be a function call. */		\
	__db_msg(dbenv, "%s\t%s", __p == NULL ? "!Set" : __p, msg);	\
} while (0)
#define	STAT_ULONG(msg, v)						\
	__db_msg(dbenv, "%lu\t%s", (u_long)(v), msg)

/*******************************************************
 * API return values
 *******************************************************/
/*
 * Return values that are OK for each different call.  Most calls have a
 * standard 'return of 0 is only OK value', but some, like db->get have
 * DB_NOTFOUND as a return value, but it really isn't an error.
 */
#define	DB_RETOK_STD(ret)	((ret) == 0)
#define	DB_RETOK_DBCDEL(ret)	((ret) == 0 || (ret) == DB_KEYEMPTY || \
				    (ret) == DB_NOTFOUND)
#define	DB_RETOK_DBCGET(ret)	((ret) == 0 || (ret) == DB_KEYEMPTY || \
				    (ret) == DB_NOTFOUND)
#define	DB_RETOK_DBCPUT(ret)	((ret) == 0 || (ret) == DB_KEYEXIST || \
				    (ret) == DB_NOTFOUND)
#define	DB_RETOK_DBDEL(ret)	DB_RETOK_DBCDEL(ret)
#define	DB_RETOK_DBGET(ret)	DB_RETOK_DBCGET(ret)
#define	DB_RETOK_DBPUT(ret)	((ret) == 0 || (ret) == DB_KEYEXIST)
#define	DB_RETOK_LGGET(ret)	((ret) == 0 || (ret) == DB_NOTFOUND)
#define	DB_RETOK_MPGET(ret)	((ret) == 0 || (ret) == DB_PAGE_NOTFOUND)
#define	DB_RETOK_REPPMSG(ret)	((ret) == 0 || \
				    (ret) == DB_REP_IGNORE || \
				    (ret) == DB_REP_ISPERM || \
				    (ret) == DB_REP_NEWMASTER || \
				    (ret) == DB_REP_NEWSITE || \
				    (ret) == DB_REP_NOTPERM || \
				    (ret) == DB_REP_STARTUPDONE)

/* Find a reasonable operation-not-supported error. */
#ifdef	EOPNOTSUPP
#define	DB_OPNOTSUP	EOPNOTSUPP
#else
#ifdef	ENOTSUP
#define	DB_OPNOTSUP	ENOTSUP
#else
#define	DB_OPNOTSUP	EINVAL
#endif
#endif

/*******************************************************
 * Files.
 *******************************************************/
/*
 * We use 1024 as the maximum path length.  It's too hard to figure out what
 * the real path length is, as it was traditionally stored in <sys/param.h>,
 * and that file isn't always available.
 */
#undef	MAXPATHLEN
#define	MAXPATHLEN	1024

#define	PATH_DOT	"."	/* Current working directory. */
				/* Path separator character(s). */
#define	PATH_SEPARATOR	"\\/:"

/*******************************************************
 * Environment.
 *******************************************************/
/* Type passed to __db_appname(). */
typedef enum {
	DB_APP_NONE=0,			/* No type (region). */
	DB_APP_DATA,			/* Data file. */
	DB_APP_LOG,			/* Log file. */
	DB_APP_TMP			/* Temporary file. */
} APPNAME;

/*
 * ALIVE_ON	The is_alive function is configured.
 * CDB_LOCKING	CDB product locking.
 * CRYPTO_ON	Security has been configured.
 * LOCKING_ON	Locking has been configured.
 * LOGGING_ON	Logging has been configured.
 * MUTEX_ON	Mutexes have been configured.
 * MPOOL_ON	Memory pool has been configured.
 * REP_ON	Replication has been configured.
 * RPC_ON	RPC has been configured.
 * TXN_ON	Transactions have been configured.
 */
#define	ALIVE_ON(dbenv)		((dbenv)->is_alive != NULL)
#define	CDB_LOCKING(dbenv)	F_ISSET(dbenv, DB_ENV_CDB)
#define	CRYPTO_ON(dbenv)	((dbenv)->crypto_handle != NULL)
#define	LOCKING_ON(dbenv)	((dbenv)->lk_handle != NULL)
#define	LOGGING_ON(dbenv)	((dbenv)->lg_handle != NULL)
#define	MPOOL_ON(dbenv)		((dbenv)->mp_handle != NULL)
#define	MUTEX_ON(dbenv)		((dbenv)->mutex_handle != NULL)
#define	REP_ON(dbenv)		((dbenv)->rep_handle != NULL)
#define	RPC_ON(dbenv)		((dbenv)->cl_handle != NULL)
#define	TXN_ON(dbenv)		((dbenv)->tx_handle != NULL)

/*
 * STD_LOCKING	Standard locking, that is, locking was configured and CDB
 *		was not.  We do not do locking in off-page duplicate trees,
 *		so we check for that in the cursor first.
 */
#define	STD_LOCKING(dbc)						\
	(!F_ISSET(dbc, DBC_OPD) &&					\
	    !CDB_LOCKING((dbc)->dbp->dbenv) && LOCKING_ON((dbc)->dbp->dbenv))

/*
 * IS_RECOVERING: The system is running recovery.
 */
#define	IS_RECOVERING(dbenv)						\
	(LOGGING_ON(dbenv) &&						\
	    F_ISSET((DB_LOG *)(dbenv)->lg_handle, DBLOG_RECOVER))

/* Initialization methods are often illegal before/after open is called. */
#define	ENV_ILLEGAL_AFTER_OPEN(dbenv, name)				\
	if (F_ISSET((dbenv), DB_ENV_OPEN_CALLED))			\
		return (__db_mi_open(dbenv, name, 1));
#define	ENV_ILLEGAL_BEFORE_OPEN(dbenv, name)				\
	if (!F_ISSET((dbenv), DB_ENV_OPEN_CALLED))			\
		return (__db_mi_open(dbenv, name, 0));

/* We're not actually user hostile, honest. */
#define	ENV_REQUIRES_CONFIG(dbenv, handle, i, flags)			\
	if (handle == NULL)						\
		return (__db_env_config(dbenv, i, flags));
#define	ENV_NOT_CONFIGURED(dbenv, handle, i, flags)			\
	if (F_ISSET((dbenv), DB_ENV_OPEN_CALLED))			\
		ENV_REQUIRES_CONFIG(dbenv, handle, i, flags)

#define	ENV_ENTER(dbenv, ip) do {					\
	int __ret;							\
	if ((dbenv)->thr_hashtab == NULL)				\
		ip = NULL;						\
	else {								\
		if ((__ret =						\
		    __env_set_state(dbenv, &(ip), THREAD_ACTIVE)) != 0)	\
			return (__ret);					\
	}								\
} while (0)

#ifdef DIAGNOSTIC
#define	ENV_LEAVE(dbenv, ip) do {					\
	if ((ip) != NULL) {						\
		DB_ASSERT(ip->dbth_state == THREAD_ACTIVE);		\
		(ip)->dbth_state = THREAD_OUT;				\
	}								\
} while (0)
#else
#define	ENV_LEAVE(dbenv, ip) do {					\
	if ((ip) != NULL)						\
		(ip)->dbth_state = THREAD_OUT;				\
} while (0)
#endif
#ifdef DIAGNOSTIC
#define	CHECK_THREAD(dbenv) do {					\
	DB_THREAD_INFO *__ip;						\
	if ((dbenv)->thr_hashtab != NULL) {				\
		(void)__env_set_state(dbenv, &__ip, THREAD_DIAGNOSTIC);	\
		DB_ASSERT(__ip != NULL &&				\
		     __ip->dbth_state != THREAD_OUT);			\
	}								\
} while (0)
#define	CHECK_MTX_THREAD(dbenv, mtx) do {				\
	if (mtx->alloc_id != MTX_MUTEX_REGION &&			\
	    mtx->alloc_id != MTX_ENV_REGION &&				\
	    mtx->alloc_id != MTX_APPLICATION)				\
		CHECK_THREAD(dbenv);					\
} while (0)
#else
#define	CHECK_THREAD(dbenv)
#define	CHECK_MTX_THREAD(dbenv, mtx)
#endif

typedef enum {
	THREAD_SLOT_NOT_IN_USE=0,
	THREAD_OUT,
	THREAD_ACTIVE,
	THREAD_BLOCKED
#ifdef DIAGNOSTIC
	, THREAD_DIAGNOSTIC
#endif
} DB_THREAD_STATE;

typedef struct __db_thread_info {
	pid_t		dbth_pid;
	db_threadid_t	dbth_tid;
	DB_THREAD_STATE	dbth_state;
	SH_TAILQ_ENTRY	dbth_links;
} DB_THREAD_INFO;

typedef struct __env_thread_info {
	u_int32_t	thr_count;
	u_int32_t	thr_max;
	u_int32_t	thr_nbucket;
	roff_t		thr_hashoff;
} THREAD_INFO;

/*******************************************************
 * Database Access Methods.
 *******************************************************/
/*
 * DB_IS_THREADED --
 *	The database handle is free-threaded (was opened with DB_THREAD).
 */
#define	DB_IS_THREADED(dbp)						\
	((dbp)->mutex != MUTEX_INVALID)

/* Initialization methods are often illegal before/after open is called. */
#define	DB_ILLEGAL_AFTER_OPEN(dbp, name)				\
	if (F_ISSET((dbp), DB_AM_OPEN_CALLED))				\
		return (__db_mi_open((dbp)->dbenv, name, 1));
#define	DB_ILLEGAL_BEFORE_OPEN(dbp, name)				\
	if (!F_ISSET((dbp), DB_AM_OPEN_CALLED))				\
		return (__db_mi_open((dbp)->dbenv, name, 0));
/* Some initialization methods are illegal if environment isn't local. */
#define	DB_ILLEGAL_IN_ENV(dbp, name)					\
	if (!F_ISSET((dbp)->dbenv, DB_ENV_DBLOCAL))			\
		return (__db_mi_env((dbp)->dbenv, name));
#define	DB_ILLEGAL_METHOD(dbp, flags) {					\
	int __ret;							\
	if ((__ret = __dbh_am_chk(dbp, flags)) != 0)			\
		return (__ret);						\
}

/*
 * Common DBC->internal fields.  Each access method adds additional fields
 * to this list, but the initial fields are common.
 */
#define	__DBC_INTERNAL							\
	DBC	 *opd;			/* Off-page duplicate cursor. */\
									\
	void	 *page;			/* Referenced page. */		\
	db_pgno_t root;			/* Tree root. */		\
	db_pgno_t pgno;			/* Referenced page number. */	\
	db_indx_t indx;			/* Referenced key item index. */\
									\
	DB_LOCK		lock;		/* Cursor lock. */		\
	db_lockmode_t	lock_mode;	/* Lock mode. */

struct __dbc_internal {
	__DBC_INTERNAL
};

/* Actions that __db_master_update can take. */
typedef enum { MU_REMOVE, MU_RENAME, MU_OPEN } mu_action;

/*
 * Access-method-common macro for determining whether a cursor
 * has been initialized.
 */
#define	IS_INITIALIZED(dbc)	((dbc)->internal->pgno != PGNO_INVALID)

/* Free the callback-allocated buffer, if necessary, hanging off of a DBT. */
#define	FREE_IF_NEEDED(sdbp, dbt)					\
	if (F_ISSET((dbt), DB_DBT_APPMALLOC)) {				\
		__os_ufree((sdbp)->dbenv, (dbt)->data);			\
		F_CLR((dbt), DB_DBT_APPMALLOC);				\
	}

/*
 * Use memory belonging to object "owner" to return the results of
 * any no-DBT-flag get ops on cursor "dbc".
 */
#define	SET_RET_MEM(dbc, owner)				\
	do {						\
		(dbc)->rskey = &(owner)->my_rskey;	\
		(dbc)->rkey = &(owner)->my_rkey;	\
		(dbc)->rdata = &(owner)->my_rdata;	\
	} while (0)

/* Use the return-data memory src is currently set to use in dest as well. */
#define	COPY_RET_MEM(src, dest)				\
	do {						\
		(dest)->rskey = (src)->rskey;		\
		(dest)->rkey = (src)->rkey;		\
		(dest)->rdata = (src)->rdata;		\
	} while (0)

/* Reset the returned-memory pointers to their defaults. */
#define	RESET_RET_MEM(dbc)				\
	do {						\
		(dbc)->rskey = &(dbc)->my_rskey;	\
		(dbc)->rkey = &(dbc)->my_rkey;		\
		(dbc)->rdata = &(dbc)->my_rdata;	\
	} while (0)

/*******************************************************
 * Mpool.
 *******************************************************/
/*
 * File types for DB access methods.  Negative numbers are reserved to DB.
 */
#define	DB_FTYPE_SET		-1		/* Call pgin/pgout functions. */
#define	DB_FTYPE_NOTSET		 0		/* Don't call... */
#define	DB_LSN_OFF_NOTSET	-1		/* Not yet set. */
#define	DB_CLEARLEN_NOTSET	UINT32_MAX	/* Not yet set. */

/* Structure used as the DB pgin/pgout pgcookie. */
typedef struct __dbpginfo {
	size_t	db_pagesize;		/* Underlying page size. */
	u_int32_t flags;		/* Some DB_AM flags needed. */
	DBTYPE  type;			/* DB type */
} DB_PGINFO;

/*******************************************************
 * Log.
 *******************************************************/
/* Initialize an LSN to 'zero'. */
#define	ZERO_LSN(LSN) do {						\
	(LSN).file = 0;							\
	(LSN).offset = 0;						\
} while (0)
#define	IS_ZERO_LSN(LSN)	((LSN).file == 0 && (LSN).offset == 0)

#define	IS_INIT_LSN(LSN)	((LSN).file == 1 && (LSN).offset == 0)
#define	INIT_LSN(LSN)		do {					\
	(LSN).file = 1;							\
	(LSN).offset = 0;						\
} while (0)

#define	MAX_LSN(LSN) do {						\
	(LSN).file = UINT32_MAX;					\
	(LSN).offset = UINT32_MAX;					\
} while (0)
#define	IS_MAX_LSN(LSN) \
	((LSN).file == UINT32_MAX && (LSN).offset == UINT32_MAX)

/* If logging is turned off, smash the lsn. */
#define	LSN_NOT_LOGGED(LSN) do {					\
	(LSN).file = 0;							\
	(LSN).offset = 1;						\
} while (0)
#define	IS_NOT_LOGGED_LSN(LSN) \
	((LSN).file == 0 && (LSN).offset == 1)

/*******************************************************
 * Txn.
 *******************************************************/
#define	DB_NONBLOCK(C)	((C)->txn != NULL && F_ISSET((C)->txn, TXN_NOWAIT))
#define	NOWAIT_FLAG(txn) \
	((txn) != NULL && F_ISSET((txn), TXN_NOWAIT) ? DB_LOCK_NOWAIT : 0)
#define	IS_SUBTRANSACTION(txn)						\
	((txn) != NULL && (txn)->parent != NULL)

/*******************************************************
 * Crypto.
 *******************************************************/
#define	DB_IV_BYTES     16		/* Bytes per IV */
#define	DB_MAC_KEY	20		/* Bytes per MAC checksum */

/*******************************************************
 * Secondaries over RPC.
 *******************************************************/
#ifdef CONFIG_TEST
/*
 * These are flags passed to DB->associate calls by the Tcl API if running
 * over RPC.  The RPC server will mask out these flags before making the real
 * DB->associate call.
 *
 * These flags must coexist with the valid flags to DB->associate (currently
 * DB_AUTO_COMMIT and DB_CREATE).  DB_AUTO_COMMIT is in the group of
 * high-order shared flags (0xff000000), and DB_CREATE is in the low-order
 * group (0x00000fff), so we pick a range in between.
 */
#define	DB_RPC2ND_MASK		0x00f00000 /* Reserved bits. */

#define	DB_RPC2ND_REVERSEDATA	0x00100000 /* callback_n(0) _s_reversedata. */
#define	DB_RPC2ND_NOOP		0x00200000 /* callback_n(1) _s_noop */
#define	DB_RPC2ND_CONCATKEYDATA	0x00300000 /* callback_n(2) _s_concatkeydata */
#define	DB_RPC2ND_CONCATDATAKEY 0x00400000 /* callback_n(3) _s_concatdatakey */
#define	DB_RPC2ND_REVERSECONCAT	0x00500000 /* callback_n(4) _s_reverseconcat */
#define	DB_RPC2ND_TRUNCDATA	0x00600000 /* callback_n(5) _s_truncdata */
#define	DB_RPC2ND_CONSTANT	0x00700000 /* callback_n(6) _s_constant */
#define	DB_RPC2ND_GETZIP	0x00800000 /* sj_getzip */
#define	DB_RPC2ND_GETNAME	0x00900000 /* sj_getname */
#endif

/*******************************************************
 * Forward structure declarations.
 *******************************************************/
struct __db_reginfo_t;	typedef struct __db_reginfo_t REGINFO;
struct __db_txnhead;	typedef struct __db_txnhead DB_TXNHEAD;
struct __db_txnlist;	typedef struct __db_txnlist DB_TXNLIST;
struct __vrfy_childinfo;typedef struct __vrfy_childinfo VRFY_CHILDINFO;
struct __vrfy_dbinfo;   typedef struct __vrfy_dbinfo VRFY_DBINFO;
struct __vrfy_pageinfo; typedef struct __vrfy_pageinfo VRFY_PAGEINFO;

#if defined(__cplusplus)
}
#endif

/*******************************************************
 * Remaining general DB includes.
 *******************************************************/


#include "dbinc/globals.h"
#include "dbinc/debug.h"
#include "dbinc/region.h"
#include "dbinc_auto/env_ext.h"
#include "dbinc/mutex.h"
#include "dbinc/os.h"
#include "dbinc/rep.h"
#include "dbinc_auto/clib_ext.h"
#include "dbinc_auto/common_ext.h"

/*******************************************************
 * Remaining Log.
 * These need to be defined after the general includes
 * because they need rep.h from above.
 *******************************************************/
/*
 * Test if the environment is currently logging changes.  If we're in recovery
 * or we're a replication client, we don't need to log changes because they're
 * already in the log, even though we have a fully functional log system.
 */
#define	DBENV_LOGGING(dbenv)						\
	(LOGGING_ON(dbenv) && !IS_REP_CLIENT(dbenv) &&			\
	    (!IS_RECOVERING(dbenv)))

/*
 * Test if we need to log a change.  By default, we don't log operations without
 * associated transactions, unless DIAGNOSTIC, DEBUG_ROP or DEBUG_WOP are on.
 * This is because we want to get log records for read/write operations, and, if
 * we trying to debug something, more information is always better.
 *
 * The DBC_RECOVER flag is set when we're in abort, as well as during recovery;
 * thus DBC_LOGGING may be false for a particular dbc even when DBENV_LOGGING
 * is true.
 *
 * We explicitly use LOGGING_ON/IS_REP_CLIENT here because we don't want to pull
 * in the log headers, which IS_RECOVERING (and thus DBENV_LOGGING) rely on, and
 * because DBC_RECOVER should be set anytime IS_RECOVERING would be true.
 *
 * If we're not in recovery (master - doing an abort a client applying
 * a txn), then a client's only path through here is on an internal
 * operation, and a master's only path through here is a transactional
 * operation.  Detect if either is not the case.
 */
#if defined(DIAGNOSTIC) || defined(DEBUG_ROP)  || defined(DEBUG_WOP)
#define	DBC_LOGGING(dbc)	__dbc_logging(dbc)
#else
#define	DBC_LOGGING(dbc)						\
	((dbc)->txn != NULL && LOGGING_ON((dbc)->dbp->dbenv) &&		\
	    !F_ISSET((dbc), DBC_RECOVER) && !IS_REP_CLIENT((dbc)->dbp->dbenv))
#endif

#endif /* !_DB_INTERNAL_H_ */
