/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
#ifndef _CONFIG_H
#define _CONFIG_H

/* Generated automatically from acconfig.h by autoheader. */
/* Please make your changes there */


/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* #undef DISABLE_SHADOW */

/* #undef HAVE_HPUX_TRUSTED_SYSTEM_PW */

/* Data types */
#define HAVE_INTXX_T 1
#define HAVE_U_INTXX_T 1
/* #undef HAVE_UINTXX_T */
/* #undef HAVE_SOCKLEN_T */
/* #undef HAVE_SIZE_T */
/* #undef HAVE_STRUCT_SOCKADDR_STORAGE */
/* #undef HAVE_STRUCT_ADDRINFO */
/* #undef HAVE_STRUCT_IN6_ADDR */
/* #undef HAVE_STRUCT_SOCKADDR_IN6 */

/* The number of bytes in a char.  */
#define SIZEOF_CHAR 1

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long int.  */
#define SIZEOF_LONG_INT 8

/* The number of bytes in a long long int.  */
#define SIZEOF_LONG_LONG_INT 8

/* The number of bytes in a short int.  */
#define SIZEOF_SHORT_INT 2

/* Define if you have the fcntl function.  */
#define HAVE_FCNTL 1

/* Define if you have the flock function.  */
#define HAVE_FLOCK 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the lockf function.  */
#define HAVE_LOCKF 1

/* Define if you have the setusercontext function.  */
/* #undef HAVE_SETUSERCONTEXT */

/* Define if you have the strcspn function.  */
#define HAVE_STRCSPN 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strlcpy function.  */
#define HAVE_STRLCPY 1

/* Define if you have the strspn function.  */
#define HAVE_STRSPN 1

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the <crypt.h> header file.  */
/* #undef HAVE_CRYPT_H */

/* Define if you have the <err.h> header file.  */
#define HAVE_ERR_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <md4.h> header file.  */
/* #undef HAVE_MD4_H */

/* Define if you have the <md5.h> header file.  */
/* #undef HAVE_MD5_H */

/* Define if you have the <md5global.h> header file.  */
/* #undef HAVE_MD5GLOBAL_H */

/* Define if you have the <rmd160.h> header file.  */
/* #undef HAVE_RMD160_H */

/* Define if you have the <sha1.h> header file.  */
/* #undef HAVE_SHA1_H */

/* Define if you have the <sys/cdefs.h> header file.  */
#define HAVE_SYS_CDEFS_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <syslog.h> header file.  */
#define HAVE_SYSLOG_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* ******************* Shouldn't need to edit below this line ************** */

#include "defines.h"

#endif /* _CONFIG_H */
