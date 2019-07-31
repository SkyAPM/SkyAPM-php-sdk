PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

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
