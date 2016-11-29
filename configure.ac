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
AC_HEADER_STDC
AC_CHECK_HEADER([argp.h], [], [AC_MSG_ERROR([argp.h not found], [1])])
AC_CHECK_HEADER([dirent.h], [], [AC_MSG_ERROR([dirent.h not found], [1])])
AC_CHECK_HEADER([fcntl.h], [], [AC_MSG_ERROR([fcntl.h not found], [1])])
AC_CHECK_HEADER([pvm3.h], [], [AC_MSG_ERROR([pvm3.h not found], [1])])
AC_CHECK_HEADER([stdio.h], [], [AC_MSG_ERROR([stdio.h not found], [1])])
AC_CHECK_HEADER([time.h], [], [AC_MSG_ERROR([time.h not found], [1])])
AC_CHECK_HEADER([unistd.h], [], [AC_MSG_ERROR([argp.h not found], [1])])
AC_CHECK_HEADER([sys/resource.h], [], [AC_MSG_ERROR([sys/resource.h not found], [1])])
AC_CHECK_HEADER([sys/stat.h], [], [AC_MSG_ERROR([sys/stat.h not found], [1])])
AC_CHECK_HEADER([sys/types.h], [], [AC_MSG_ERROR([sys/types.h not found], [1])])
AC_CHECK_HEADER([sys/wait.h], [], [AC_MSG_ERROR([sys/wait.h not found], [1])])

# Check for typedefs, structures, and compiler characteristics

# Check for library functions

# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
