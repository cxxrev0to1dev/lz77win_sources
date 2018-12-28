AC_INIT([ckcore],[0.1])
AM_INIT_AUTOMAKE([-Wall -Werror])
LT_INIT
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# These compilation flags must be set before AC_PROG_CC/CXX,
# or we'll get "-g -O2" by default.
CFLAGS="${CFLAGS=} -g3 -Wall"
CXXFLAGS="${CXXFLAGS=} -g3 -Wall"

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CONFIG_MACRO_DIR([m4])

# Version information (current:revision:age).
CKCORE_VERSION=1:0:0
AC_SUBST(CKCORE_VERSION)

AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

# check if debug or release build.
AC_MSG_CHECKING(whether to generate a debug build)
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug=[[yes/no]]],
                              [generates a debug build [default=no]])],
              [case "${enableval}" in
               yes) debug_build=true ;;
               no)  debug_build=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
               esac],
              debug_build=false)

if [ test x$debug_build = xtrue ]
then
    AC_MSG_RESULT(yes)
    CXXFLAGS="$CXXFLAGS -O0 -DDEBUG"
else
    AC_MSG_RESULT(no)
    # If we let the 'omit frame pointer' optimisation enabled,
    # we'll have difficulty later on while trying
    # to analyse crash dump files sent by the users.
    CXXFLAGS="$CXXFLAGS -O3 -DNDEBUG -fno-omit-frame-pointer"
fi

AC_DEFINE(_UNIX)
AC_DEFINE(_FILE_OFFSET_BIT,64,[Enable support for large files.])

AC_CONFIG_FILES([Makefile src/Makefile src/unix/Makefile])
AC_OUTPUT

