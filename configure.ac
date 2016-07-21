m4_define([VLMC_MAJOR], [0])
m4_define([VLMC_MINOR], [2])
m4_define([VLMC_MICRO], [0])
m4_define([VLMC_CODENAME], ["alpha"])

COPYRIGHT_YEARS="2008-2016"
AC_COPYRIGHT([Copyright $COPYRIGHT_YEARS VideoLAN])

AC_INIT([vlmc], [VLMC_MAJOR.VLMC_MINOR.VLMC_MICRO])
AC_LANG(C++)

AM_INIT_AUTOMAKE([foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES], [
  AM_SILENT_RULES([yes])
])
AC_CANONICAL_HOST
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext])
AX_CHECK_COMPILE_FLAG([-fPIC], [PIC_FLAGS=-fPIC])

CXXFLAGS="${CXXFLAGS} ${PIC_FLAGS}"

AC_CONFIG_MACRO_DIR([m4])

dnl
dnl  Check the operating system
dnl
HAVE_WIN64="0"
HAVE_IOS="0"
HAVE_OSX="0"

case "${host_os}" in
  "")
    SYS=unknown
    ;;
  linux*)
    SYS=linux
    ;;
  bsdi*)
    SYS=bsdi
    ;;
  freebsd*)
    SYS=freebsd
    CPPFLAGS="${CPPFLAGS} -I/usr/local/include"
    LDFLAGS="${LDFLAGS} -L/usr/local/lib"
    ;;
  openbsd*)
    SYS=openbsd
    CPPFLAGS="${CPPFLAGS} -I/usr/local/include -I/usr/X11R6/include"
    LDFLAGS="${LDFLAGS} -L/usr/local/lib -L/usr/X11R6/lib/"
    ;;
  netbsd*)
    SYS=netbsd
    ;;
  dragonfly*)
    SYS=dragonfly
    ;;
  darwin*)

    dnl Force gcc "-arch" flag
    ARCH_flag=""
    case "${host}" in
      i?86*)
        ARCH_flag="-arch i386"
      ;;
      ppc64*)
        ARCH_flag="-arch ppc64"
      ;;
      ppc*)
        ARCH_flag="-arch ppc"
      ;;
      x86_64*)
        ARCH_flag="-arch x86_64"
      ;;
      arm*)
        ac_cv_c_bigendian="no"
        ac_cv_c_attribute_packed="no"
      ;;
    esac

    SYS=darwin
    CFLAGS="${CFLAGS} -D_INTL_REDIRECT_MACROS -std=gnu11 ${ARCH_flag}"
    CXX="${CXX} -stdlib=libc++ -std=c++11"
    CXXFLAGS="${CXXFLAGS} -D_INTL_REDIRECT_MACROS ${ARCH_flag}"
    CPPFLAGS="${CPPFLAGS} ${ARCH_flag}"
    OBJCFLAGS="${OBJCFLAGS} -D_INTL_REDIRECT_MACROS -std=gnu11 ${ARCH_flag}"
    LDFLAGS="${LDFLAGS} -Wl,-headerpad_max_install_names ${ARCH_flag}"

    AC_EGREP_CPP(yes,
            [#import <TargetConditionals.h>
             #if TARGET_OS_IPHONE
             yes
             #endif],
             [HAVE_IOS="1"],
             [HAVE_OSX="1"])

    dnl Allow binaries created on Lion to run on earlier releases
    AC_EGREP_CPP(yes,
           [#import <Cocoa/Cocoa.h>
            #ifdef MAC_OS_X_VERSION_10_7
            yes
            #endif],
            [AC_MSG_RESULT([yes])
            AC_LIBOBJ([getdelim])
            AC_LIBOBJ([strndup])
            AC_LIBOBJ([strnlen])],)

    dnl
    dnl  Handle Mac OS X SDK flags
    dnl
    AC_ARG_WITH(macosx-sdk,
      [AS_HELP_STRING([--with-macosx-sdk=DIR],
        [compile using the SDK in DIR])])
    if test "${with_macosx_sdk}" != "" ; then
        test ! -d "${with_macosx_sdk}" && AC_MSG_ERROR([SDK "${with_macosx_sdk}" not found])
        CPP="${CPP} -isysroot ${with_macosx_sdk}"
        CC="${CC} -isysroot ${with_macosx_sdk}"
        CXX="${CXX} -isysroot ${with_macosx_sdk}"
        OBJC="${OBJC} -isysroot ${with_macosx_sdk}"
        LD="${LD} -syslibroot ${with_macosx_sdk}"
    fi
    AC_ARG_WITH(macosx-version-min,
      [AS_HELP_STRING([--with-macosx-version-min=VERSION],
        [compile for Mac OS X VERSION and above])])
    if test "${with_macosx_version_min}" != "" ; then
        CPP="${CPP} -mmacosx-version-min=${with_macosx_version_min}"
        CC="${CC} -mmacosx-version-min=${with_macosx_version_min}"
        CXX="${CXX} -mmacosx-version-min=${with_macosx_version_min}"
        OBJC="${OBJC} -mmacosx-version-min=${with_macosx_version_min}"
        LD="${LD} -mmacosx_version_min=${with_macosx_version_min}"
    fi
    ;;
  *mingw32* | *cygwin* | *wince* | *mingwce*)
    AC_DEFINE([_UNICODE], [1], [Define to 1 for Unicode (Wide Chars) APIs.])
    AC_DEFINE([UNICODE], [1], [Define to 1 for Unicode (Wide Chars) APIs.])
    AC_DEFINE([_POSIX_C_SOURCE], [200809L], [IEEE Std 1003.1.])
    AC_DEFINE([_BSD_SOURCE], [1], [ISO C, POSIX, and 4.3BSD things.])

    case "${host_os}" in
      *mingw32*)
        SYS=mingw32
        ;;
      *cygwin*)
        SYS=cygwin
        ;;
    esac

    if test "${SYS}" = "mingw32"; then
        # DEP, ASLR, NO SEH
        LDFLAGS="${LDFLAGS} -Wl,--nxcompat -Wl,--no-seh -Wl,--dynamicbase"

        AC_CHECK_PROGS(U2D, [unix2dos todos], unix2dos)
        ac_default_prefix="`pwd`/_win32"
        DESTDIR="`pwd`/_win32/"

        dnl
        dnl NSIS/MSI Installer prefix and WIN64
        dnl
        case "${host}" in
            amd64*|x86_64*)
                HAVE_WIN64="1"
                WINDOWS_ARCH="x64"
                PROGRAMFILES="PROGRAMFILES64"
            ;;
            *)
                WINDOWS_ARCH="x86"
                PROGRAMFILES="PROGRAMFILES"
            ;;
        esac
        AC_SUBST(WINDOWS_ARCH)
        AC_SUBST(PROGRAMFILES)

    fi
    ;;
  *nto*)
    SYS=nto
    ;;
  solaris*)
    SYS=solaris
    ;;
  hpux*)
    SYS=hpux
    ;;
  *os2*)
    SYS=os2
    LDFLAGS="${LDFLAGS} -Zomf -Zbin-files -Zargs-wild -Zhigh-mem"
    AC_LIBOBJ([freeaddrinfo])
    ;;
  *)
    SYS="${host_os}"
    ;;
esac
AM_CONDITIONAL(HAVE_WIN32,   test "${SYS}" = "mingw32")
AM_CONDITIONAL(HAVE_LINUX,   test "${SYS}" = "linux")
AM_CONDITIONAL(HAVE_DARWIN,  test "${SYS}" = "darwin")
AM_CONDITIONAL(HAVE_IOS,     test "${HAVE_IOS}" = "1")
AM_CONDITIONAL(HAVE_OSX,     test "${HAVE_OSX}" = "1")

dnl Android is linux, but a bit different
AS_IF([test "$SYS" = linux],[
    AC_MSG_CHECKING([for an Android system])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
                      [[#ifndef __ANDROID__
                       # error Not Android
                       #endif
                       ]],[[;]])
        ],[
            HAVE_ANDROID="1"
            AC_MSG_RESULT([yes])
        ],[AC_MSG_RESULT([no])
    ])
])
AM_CONDITIONAL(HAVE_ANDROID, test "${HAVE_ANDROID}" = "1")

AC_MSG_CHECKING([for 3rd party libraries path])
AS_IF([test -z "${with_contrib}" || test "${with_contrib}" = "yes"], [
    CONTRIB_DIR="${srcdir}/contribs/${host}"
    AS_IF([test ! -d "${CONTRIB_DIR}"], [
        echo "${CONTRIB_DIR} not found" >&AS_MESSAGE_LOG_FD
        CONTRIB_DIR="${srcdir}/contribs/`$CXX -dumpmachine`"
        AS_IF([test ! -d "${CONTRIB_DIR}"], [
            echo "${CONTRIB_DIR} not found" >&AS_MESSAGE_LOG_FD
        ])
    ])
])

AS_IF([test -n "${CONTRIB_DIR}"], [
    AS_IF([test -d "${CONTRIB_DIR}/lib"],[
        CONTRIB_DIR=`cd "${CONTRIB_DIR}" && pwd`
        AC_MSG_RESULT(["${CONTRIB_DIR}"])
        export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${CONTRIB_DIR}/lib/pkgconfig"
    ], [
        echo "${CONTRIB_DIR}/lib not found" >&AS_MESSAGE_LOG_FD
        CONTRIB_DIR=""
        AC_MSG_RESULT([not usable])
    ])
])


AC_ARG_ENABLE(gui, AC_HELP_STRING([--disable-gui],[Disable VLMC GUI]))
AM_CONDITIONAL(HAVE_GUI, [test "${enable_gui}" !=  "no"])
AS_IF([test "${enable_gui}" != "no"],[
    AC_DEFINE(HAVE_GUI, 1, [Define to 1 for GUI support])
])
AC_ARG_ENABLE(crashhandler, AC_HELP_STRING([--enable-crashhandler],[Enable VLMC crash handler]))
AM_CONDITIONAL(HAVE_CRASHHANDLER, [test "${enable_crashhandler}" = "yes"])

#FIXME: Don't check for QtGui/Qt5Quick when building without GUI
PKG_CHECK_MODULES(QT, [Qt5Core Qt5Widgets Qt5Gui Qt5Network Qt5Quick], [
   QT_PATH="$(eval $PKG_CONFIG --variable=exec_prefix Qt5Core)"
   QT_HOST_PATH="$(eval $PKG_CONFIG --variable=host_bins Qt5Core)"
   AC_PATH_PROGS(MOC, [moc-qt5 moc], moc, ["${QT_HOST_PATH}" "${QT_PATH}/bin"])
   AC_PATH_PROGS(RCC, [rcc-qt5 rcc], rcc, ["${QT_HOST_PATH}" "${QT_PATH}/bin"])
   AC_PATH_PROGS(UIC, [uic-qt5 uic], uic, ["${QT_HOST_PATH}" "${QT_PATH}/bin"])
])
PKG_CHECK_MODULES(VLC, libvlc >= 3.0)
PKG_CHECK_MODULES(VLCPP, libvlcpp)
PKG_CHECK_MODULES(MLT, mlt-framework >= 6.3)
PKG_CHECK_MODULES(MLTPP, mlt++ >= 6.3.0)

COPYRIGHT_MESSAGE="Copyright © ${COPYRIGHT_YEARS} the VideoLAN team"
AC_DEFINE_UNQUOTED(CODENAME, VLMC_CODENAME, [Package codename])
AC_DEFINE_UNQUOTED(VLMC_COMPILE_BY, "`whoami|sed -e 's/\\\/\\\\\\\/g'`", [user who ran configure])
AC_DEFINE_UNQUOTED(VLMC_COMPILE_HOST, "`hostname -f 2>/dev/null || hostname`", [hostname on which configure was ran])
AC_DEFINE_UNQUOTED(VLMC_COMPILE_SYSTEM, "`uname -rs 2>/dev/null || echo unknown`", [host type that ran configure])
AC_DEFINE_UNQUOTED(VLMC_COMPILER, "`$CC -v 2>&1 | tail -n 1 | sed -e 's/ *$//'`", [compiler])
AC_DEFINE_UNQUOTED(COPYRIGHT_MESSAGE, "${COPYRIGHT_MESSAGE}", [Copyright string])
AC_DEFINE_UNQUOTED(PROJECT_CONTACT, "vlmc-devel@videolan.org", [Contact mailing list])
AC_DEFINE_UNQUOTED(PROJECT_WEBSITE, "www.videolan.org", [Organisation website])

PKG_INSTALLDIR

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT

