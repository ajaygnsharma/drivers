#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([timer_ibuc], [1.0], [rmcdonald@terrasatinc.com])
AM_INIT_AUTOMAKE([foreign])
IBUC_DIR=$(top_srcdir)
IBUC_CPPFLAGS=""

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])
AC_SEARCH_LIBS([timer_getoverrun], [rt])
AC_SEARCH_LIBS([timer_create], [rt])
AC_SEARCH_LIBS([timer_settime], [rt])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
