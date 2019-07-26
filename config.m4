PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

PHP_ARG_ENABLE(mysqlnd, whether mysqli use mysqlnd,
[  --enable-mysqlnd           Whether mysqli use mysql(ARG=yes)], yes, no)

if test "$PHP_SKYWALKING" != "no"; then
  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      decode.c \
      encode.c \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

if test "$PHP_MYSQLND" != "no"; then
    AC_DEFINE(MYSQLI_USE_MYSQLND, 1, [Flag mysqli use mysqlnd])
fi