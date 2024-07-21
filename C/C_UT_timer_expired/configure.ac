dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ut_timer_expired, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CC

AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])
AC_SEARCH_LIBS([timer_getoverrun], [rt])
AC_SEARCH_LIBS([timer_create], [rt])
AC_SEARCH_LIBS([timer_settime], [rt])

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

