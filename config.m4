PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

if test "$PHP_SKYWALKING" != "no"; then
  PHP_ADD_INCLUDE("src")
  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      src/decode.c \
      src/encode.c \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
