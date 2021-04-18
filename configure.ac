AC_PREREQ([2.69])
AC_INIT([pisi-timedate], [1.1], [belka.ew@gmail.com])
AC_CONFIG_SRCDIR([pisi-timedate.h])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CC

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])

PKG_CHECK_MODULES([GDBUS], [gio-2.0 dbus-1])

AC_OUTPUT
