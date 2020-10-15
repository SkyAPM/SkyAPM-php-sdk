PHP_REQUIRE_CXX()
CXXFLAGS="$CXXFLAGS -Wall -std=c++11 -Wno-deprecated-register"

PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking           Enable skywalking support])

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

  AC_PATH_PROG(PROTOC, protoc, no)
  if ! test -x "$PROTOC"; then
    AC_MSG_ERROR([protoc command missing, please reinstall the protobuf distribution])
  fi
  AC_PATH_PROG(GRPC_CPP_PLUGIN, grpc_cpp_plugin, no)
  if ! test -x "$GRPC_CPP_PLUGIN"; then
    AC_MSG_ERROR([grpc_cpp_plugin command missing, please reinstall the grpc distribution])
  fi

  mkdir -p src/network/v3
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/common/Common.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/language-agent/*.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/profile/*.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/management/*.proto
  find src -name "*.grpc.pb.cc" | while read id; do mv $id ${id/.grpc/_grpc}; done

  PROTOBUF_LIBDIR=$PROTOBUF_DIR/${PROTOBUF_LIB_SUBDIR-lib}
  PHP_ADD_LIBPATH($PROTOBUF_LIBDIR)
  PHP_ADD_LIBRARY(protobuf,,SKYWALKING_SHARED_LIBADD)
  AC_DEFINE(HAVE_PROTOBUFLIB,1,[ ])

  PHP_SUBST(SKYWALKING_SHARED_LIBADD)

  PHP_ADD_INCLUDE(src)
  PHP_ADD_INCLUDE(src/network/v3)


  PHP_NEW_EXTENSION(skywalking, \
      skywalking.cc \
      src/base64.cc \
      src/cross_process_bag.cc \
      src/manager.cc \
      src/segment.cc \
      src/segment_reference.cc \
      src/sky_curl.cc \
      src/sky_execute.cc \
      src/sky_grpc.cc \
      src/sky_predis.cc \
      src/sky_module.cc \
      src/sky_pdo.cc \
      src/sky_plugin_rabbit_mq.cc \
      src/sky_utils.cc \
      src/span.cc \
      src/tag.cc \
      src/network/v3/common/Common_grpc.pb.cc \
      src/network/v3/common/Common.pb.cc \
      src/network/v3/language-agent/Tracing.pb.cc \
      src/network/v3/language-agent/Tracing_grpc.pb.cc \
      src/network/v3/management/Management_grpc.pb.cc \
      src/network/v3/management/Management.pb.cc \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1, cxx)
fi

if test -r $phpincludedir/ext/mysqli/mysqli_mysqlnd.h; then
    AC_DEFINE([MYSQLI_USE_MYSQLND], 1, [Whether mysqlnd is enabled])
fi
