PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

PHP_ARG_ENABLE(mysqli, whether to enable mysqli support,
[  --enable-mysqli           Enable mysqli support], yes, yes)

if test "$PHP_SKYWALKING" != "no"; then
  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      decode.c \
      encode.c \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

if test "$PHP_MYSQLI" != "no"; then
    AC_DEFINE(SKYWALKING_ENABLED_MYSQLI, 1, [Enable skywalking support])
fi
