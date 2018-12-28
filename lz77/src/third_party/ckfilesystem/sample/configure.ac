AC_INIT
AM_INIT_AUTOMAKE(ckfs_sample,0.1)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

# With arguments.
AC_ARG_WITH([ckcore],
            [AS_HELP_STRING([--with-ckcore],
         				    [installation prefix of ckcore])],
            [with_ckcore=$withval],
            [with_ckcore=no])

AC_ARG_WITH([ckfilesystem],
            [AS_HELP_STRING([--with-ckfilesystem],
         				    [installation prefix of ckfilesystem])],
            [with_ckfilesystem=$withval],
            [with_ckfilesystem=no])

if test -d "$with_ckcore"
then
	CXXFLAGS="$CXXFLAGS -I$with_ckcore/include"
	LIBS="$LIBS -L$with_ckcore/lib -lckcore"
fi

if test -d "$with_ckfilesystem"
then
	CXXFLAGS="$CXXFLAGS -I$with_ckfilesystem/include"
	LIBS="$LIBS -L$with_ckfilesystem/lib -lckfilesystem"
fi

AS_IF([test "x$with_ckcore" != xno],
	  [AC_CHECK_LIB([ckcore],[main],[],
              		[AC_MSG_FAILURE([--with-ckcore was given, but test for ckcore failed])])])

AS_IF([test "x$with_ckfilesystem" != xno],
	  [AC_CHECK_LIB([ckfilesystem],[main],[],
              		[AC_MSG_FAILURE([--with-ckfilesystem was given, but test for ckfilesystem failed])])])

AC_DEFINE(_UNIX)
AC_DEFINE(_FILE_OFFSET_BIT,64,[Enable support for large files.])

AC_OUTPUT(Makefile src/Makefile)

