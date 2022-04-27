PHP_ARG_ENABLE([skywalking],
  [whether to enable skywalking support],
  [AS_HELP_STRING([--enable-skywalking],
    [Enable skywalking support])],
  [yes])

if test "$PHP_THREAD_SAFETY" == "yes"; then
  AC_MSG_ERROR([skywalking does not support ZTS])
fi

AC_MSG_CHECKING([for php_json.h])
json_inc_path=""
if test -f "$abs_srcdir/include/php/ext/json/php_json.h"; then
  json_inc_path="$abs_srcdir/include/php"
elif test -f "$abs_srcdir/ext/json/php_json.h"; then
  json_inc_path="$abs_srcdir"
elif test -f "$phpincludedir/ext/json/php_json.h"; then
  json_inc_path="$phpincludedir"
else
  for i in php php7; do
    if test -f "$prefix/include/$i/ext/json/php_json.h"; then
      json_inc_path="$prefix/include/$i"
    fi
  done
fi

if test "$json_inc_path" = ""; then
  AC_MSG_ERROR([Could not fond php_json.h, please reinstall the php-json extension])
else
  AC_MSG_RESULT([found in $json_inc_path])
fi


if test "$PHP_SKYWALKING" != "no"; then
  AC_PATH_PROG(GO, go, no)
  if ! test -x "$GO"; then
    AC_MSG_ERROR([go command missing, please reinstall the go distribution])
  fi
  go build -buildmode=c-archive -o src src/sky_go_wrapper.go

  EXTRA_LDFLAGS="$EXTRA_LDFLAGS src/sky_go_wrapper.a"
  LIBS="-lpthread $LIBS"

  SKYWALKING_SHARED_LIBADD="-lpthread $SKYWALKING_SHARED_LIBADD"
  PHP_ADD_LIBRARY(pthread)
  PHP_ADD_LIBRARY(dl,,SKYWALKING_SHARED_LIBADD)
  PHP_ADD_LIBRARY(dl)

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
