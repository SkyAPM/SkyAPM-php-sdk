PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

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
  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      decode.c \
      encode.c \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

if test -r $phpincludedir/ext/mysqli/mysqli_mysqlnd.h; then
    AC_DEFINE([MYSQLI_USE_MYSQLND], 1, [Whether mysqlnd is enabled])
fi
