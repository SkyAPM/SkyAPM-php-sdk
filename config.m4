PHP_REQUIRE_CXX()
CXXFLAGS="$CXXFLAGS -Wall -std=c++11 -Wno-deprecated-register"

PHP_ARG_ENABLE([skywalking],
  [whether to enable skywalking support],
  [AS_HELP_STRING([--enable-skywalking],
    [Enable skywalking support])],
  [yes])

if test "$PHP_THREAD_SAFETY" == "yes"; then
  AC_MSG_ERROR([skywalking does not support ZTS])
fi

AC_LANG_PUSH(C++)
AX_CHECK_COMPILE_FLAG([-std=c++0x], , [AC_MSG_ERROR([compiler not accept c++11])])
AC_LANG_POP()

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

  case $host in
    *darwin*)
      PHP_ADD_LIBRARY(c++,1,SKYWALKING_SHARED_LIBADD)
      ;;
    *)
      PHP_ADD_LIBRARY(rt,,SKYWALKING_SHARED_LIBADD)
      PHP_ADD_LIBRARY(rt)
      ;;
  esac

  PHP_SUBST(SKYWALKING_SHARED_LIBADD)

  PHP_ADD_INCLUDE(src)

  PHP_NEW_EXTENSION(skywalking, \
      skywalking.cc \
      src/base64.cc \
      src/sky_core_cross_process.cc \
      src/sky_core_log.cc \
      src/sky_core_segment.cc \
      src/sky_core_segment_reference.cc \
      src/sky_core_span.cc \
      src/sky_core_tag.cc \
      src/sky_execute.cc \
      src/sky_go_utils.cc \
      src/sky_log.cc \
      src/sky_module.cc \
      src/sky_plugin_mysqli.cc \
      src/sky_pdo.cc \
      src/sky_plugin_curl.cc \
      src/sky_plugin_error.cc \
      src/sky_plugin_grpc.cc \
      src/sky_plugin_hyperf_guzzle.cc \
      src/sky_plugin_predis.cc \
      src/sky_plugin_rabbit_mq.cc \
      src/sky_plugin_redis.cc \
      src/sky_plugin_memcached.cc \
      src/sky_plugin_yar.cc \
      src/sky_plugin_swoole_curl.cc \
      src/sky_rate_limit.cc \
      src/sky_utils.cc \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1, cxx)
fi

if test -r $phpincludedir/ext/mysqli/mysqli_mysqlnd.h; then
    AC_DEFINE([MYSQLI_USE_MYSQLND], 1, [Whether mysqlnd is enabled])
fi
