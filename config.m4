PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

PHP_ARG_ENABLE(yarclient, whether to enable yar client support,
[  --enable-yarclient           Enable yar client support], no, no)

if test "$PHP_SKYWALKING" != "no"; then
  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      decode.c \
      encode.c \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

if test "$PHP_YARCLIENT" != "no"; then
  AC_DEFINE(ENABLE_YAR_CLIENT, 1, [Enable yar client support])
fi
