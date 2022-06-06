PHP_ARG_ENABLE([skywalking],
  [whether to enable skywalking support],
  [AS_HELP_STRING([--enable-skywalking],
    [Enable skywalking support])],
  [yes])

if test "$PHP_THREAD_SAFETY" == "yes"; then
  AC_MSG_ERROR([skywalking does not support ZTS])
fi

if test "$PHP_SKYWALKING" != "no"; then
  AC_PATH_PROG(CARGO, cargo, no)
  if ! test -x "$CARGO"; then
    AC_MSG_ERROR([cargo command missing, please reinstall the cargo distribution])
  fi
  AC_PATH_PROG(RUSTFMT, rustfmt, no)
  if ! test -x "$RUSTFMT"; then
    AC_MSG_ERROR([rustfmt command missing, please reinstall the cargo distribution])
  fi

  EXTRA_LDFLAGS="$EXTRA_LDFLAGS target/release/libsky_core_report.a"
  LIBS="-lpthread $LIBS"

  SKYWALKING_SHARED_LIBADD="-lpthread $SKYWALKING_SHARED_LIBADD"
  PHP_ADD_LIBRARY(pthread)
  PHP_ADD_LIBRARY(dl,,SKYWALKING_SHARED_LIBADD)
  PHP_ADD_LIBRARY(dl)
  case $host in
    *darwin*)
      ;;
    *)
      PHP_ADD_LIBRARY(rt,,SKYWALKING_SHARED_LIBADD)
      PHP_ADD_LIBRARY(rt)
      ;;
  esac

  PHP_SUBST(SKYWALKING_SHARED_LIBADD)

  PHP_ADD_INCLUDE(src)
  AC_DEFINE(HAVE_SKYWALKING, 1, [ Have skywalking support ])

  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      src/sky_core_cross_process.c \
      src/sky_core_log.c \
      src/sky_core_module.c \
      src/sky_core_segment.c \
      src/sky_core_segment_reference.c \
      src/sky_core_span.c \
      src/sky_core_tag.c \
      src/sky_plugin_curl.c \
      src/sky_plugin_redis.c \
      src/sky_util_base64.c \
      src/sky_util_php.c \
      src/sky_utils.c \
  , $ext_shared)
fi

if test -r $phpincludedir/ext/mysqli/mysqli_mysqlnd.h; then
    AC_DEFINE([MYSQLI_USE_MYSQLND], 1, [Whether mysqlnd is enabled])
fi
shared_objects_skywalking="target/release/libsky_core_report.a $shared_objects_skywalking"
dnl PHP_SUBST(shared_objects_skywalking)
case $host in
    *darwin*)
      echo "target/release/libsky_core_report.a:\n	cargo build --release" >> Makefile.objects
      ;;
    *)
      echo -e "target/release/libsky_core_report.a:\n	cargo build --release" >> Makefile.objects
      ;;
esac