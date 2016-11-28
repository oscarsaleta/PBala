# Prelude
AC_INIT([PBala], [4.1.0], [osr@mat.uab.cat], [], [https://github.com/oscarsaleta/PBala])
AC_REVISION([$Revision: 1.0$])
AC_CONFIG_SRCDIR([src/PBala.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Check for C compiler
AC_PROG_CC

# Check for libraries
AC_CHECK_LIB([m],[sin],[], [AC_MSG_ERROR([Math library [(-lm)] not found], [1])])
AC_CHECK_LIB([pvm3],[pvm_start_pvmd], [], [AC_MSG_ERROR([PVM library [(-lpvm3)] not found], [1])])

# Check for header files
AC_CHECK_HEADER([argp.h], [], [AC_MSG_ERROR([argp.h not found], [1])])
AC_CHECK_HEADER([dirent.h], [], [AC_MSG_ERROR([dirent.h not found], [1])])
AC_CHECK_HEADER([stdio.h], [], [AC_MSG_ERROR([stdio.h not found], [1])])
AC_CHECK_HEADER([stdlib.h], [], [AC_MSG_ERROR([stdlib.h not found], [1])])
AC_CHECK_HEADER([string.h], [], [AC_MSG_ERROR([string.h not found], [1]) ])
AC_CHECK_HEADER([time.h], [], [AC_MSG_ERROR([time.h not found], [1])])
AC_CHECK_HEADER([unistd.h], [], [AC_MSG_ERROR([argp.h not found], [1])])

# Check for typedefs, structures, and compiler characteristics

# Check for library functions

# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
