DIALECT="-std=c++11"
echo 'int main() {return 0;}' > ./log.cpp && $CXX -std=c++11 ./log.cpp || $DIALECT="no"

if test $DILAECT = no; then
    AC_MSG_ERROR([c++ compiler does not support c++11])
else
    echo $DILAECT
fi

PHP_REQUIRE_CXX()
CXXFLAGS="$CXXFLAGS -Wall -std=c++11"

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

  PHP_ADD_INCLUDE(src)
  PHP_ADD_INCLUDE(src/network/v3)

  SEARCH_PATH="/usr/local /usr"
  SEARCH_GRPC_FOR="/include/grpc/grpc.h"
  SEARCH_PROTOBUF_FOR="/include/google/protobuf/message.h"

  AC_MSG_CHECKING([for grpc and protobuf files in default path])
  for i in $SEARCH_PATH ; do
    if test -r $i/$SEARCH_GRPC_FOR; then
      GRPC_DIR=$i
      AC_MSG_RESULT(found in $i)
    fi

    if test -r $i/$SEARCH_PROTOBUF_FOR; then
      PROTOBUF_DIR=$i
      AC_MSG_RESULT(found in $i)
    fi
  done

  if test -z "$GRPC_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the grpc distribution])
  fi

  if test -z "$PROTOBUF_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the protobuf distribution])
  fi

  PHP_ADD_INCLUDE($GRPC_DIR/include)
  PHP_ADD_INCLUDE($PROTOBUF_DIR/include)
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
      PHP_ADD_LIBRARY(stdc++,1,SKYWALKING_SHARED_LIBADD)
      PHP_ADD_LIBRARY(rt,,SKYWALKING_SHARED_LIBADD)
      PHP_ADD_LIBRARY(rt)
      ;;
  esac



  GRPC_LIBDIR=$GRPC_DIR/${GRPC_LIB_SUBDIR-lib}
  PHP_ADD_LIBPATH($GRPC_LIBDIR)

  PHP_CHECK_LIBRARY(gpr, gpr_now,
  [
    PHP_ADD_LIBRARY(gpr,, SKYWALKING_SHARED_LIBADD)
    PHP_ADD_LIBRARY(gpr)
    AC_DEFINE(HAVE_GPRLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong gpr lib version or lib not found])
  ],[
    -L$GRPC_LIBDIR
  ])

  PHP_CHECK_LIBRARY(grpc, grpc_channel_destroy,
  [
    PHP_ADD_LIBRARY(grpc,,SKYWALKING_SHARED_LIBADD)
    PHP_ADD_LIBRARY(grpc++,,SKYWALKING_SHARED_LIBADD)
    AC_DEFINE(HAVE_GRPCLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong grpc lib version or lib not found])
  ],[
    -L$GRPC_LIBDIR
  ])

  PROTOBUF_LIBDIR=$PROTOBUF_DIR/${PROTOBUF_LIB_SUBDIR-lib}
  PHP_ADD_LIBPATH($PROTOBUF_LIBDIR)
  PHP_ADD_LIBRARY(protobuf,,SKYWALKING_SHARED_LIBADD)
  AC_DEFINE(HAVE_PROTOBUFLIB,1,[ ])

  PHP_SUBST(SKYWALKING_SHARED_LIBADD)


  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      decode.c \
      encode.c \
      src/base64.cc \
      src/manager.cc \
      src/manager_wrapper.cc \
      src/segment.cc \
      src/segment_wrapper.cc \
      src/span.cc \
      src/span_wrapper.cc \
      src/network/v3/common/Common_grpc.pb.cc \
      src/network/v3/common/Common.pb.cc \
      src/network/v3/management/Management_grpc.pb.cc \
      src/network/v3/management/Management.pb.cc \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

if test -r $phpincludedir/ext/mysqli/mysqli_mysqlnd.h; then
    AC_DEFINE([MYSQLI_USE_MYSQLND], 1, [Whether mysqlnd is enabled])
fi
