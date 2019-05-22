dnl $Id$
dnl config.m4 for extension skywalking

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(skywalking, for skywalking support,
dnl Make sure that the comment is aligned:
dnl [  --with-skywalking             Include skywalking support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
dnl Make sure that the comment is aligned:
dnl [  --enable-skywalking           Enable skywalking support])


PHP_ARG_ENABLE(skywalking, whether to enable skywalking support,
[  --enable-skywalking     Enable skywalking support])


PHP_ARG_WITH(libgrpc, for libgrpc support,
[  --with-libgrpc[=DIR]      Include libxyz support])


if test -z "$PHP_DEBUG"; then
    AC_ARG_ENABLE(debug,
        [  --enable-debug compile with debugging system],
        [PHP_DEBUG=$enableval], [PHP_DEBUG=no]
    )
fi

if test "$PHP_SKYWALKING" != "no"; then
  PHP_REQUIRE_CXX()

  dnl search grpc
  SEARCH_GRPC_PATH="/usr/local /usr"
  SEARCH_GRPC_FOR="/include/grpc/grpc.h"

  if test -r $PHP_LIBGRPC/$SEARCH_GRPC_FOR; then
    LIBGRPC_DIR=$PHP_LIBGRPC
  else
    AC_MSG_CHECKING([for libgrpc files in default path])
    for i in $SEARCH_GRPC_PATH ; do
      if test -r $i/$SEARCH_GRPC_FOR; then
        LIBGRPC_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$LIBGRPC_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please install libgrpc - See https://github.com/grpc/grpc])
  fi

  PHP_ADD_INCLUDE($LIBGRPC_DIR/include)
  PHP_ADD_INCLUDE(src/report/deps/boost)

  PHP_CHECK_LIBRARY(grpc, grpc_channel_destroy,
  [
    PHP_ADD_LIBRARY_WITH_PATH(grpc, $LIBGRPC_DIR/lib, LIBGRPC_SHARED_LIBADD)
    AC_DEFINE(HAVE_LIBGRPCLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libgrpc lib version or lib not found])
  ],[
    -L$LIBGRPC_DIR/lib
  ])

  PHP_SUBST(LIBGRPC_SHARED_LIBADD)

  dnl SKYWALKING_SHARED_LIBADD="-lpthread $SKYWALKING_SHARED_LIBADD"
  dnl AC_MSG_ERROR($SKYWALKING_SHARED_LIBADD)
  dnl KYWALKING_LIBS=`pkg-config --cflags --libs protobuf grpc++ grpc`
  dnl KYWALKING_LIBS+=" -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl "

  AC_OUTPUT_COMMANDS(
    protoc -I ./src/protocol-6 --cpp_out=./src/grpc ./src/protocol-6/common/*.proto
    protoc -I ./src/protocol-6 --cpp_out=./src/grpc ./src/protocol-6/register/*.proto
    protoc -I ./src/protocol-6 --cpp_out=./src/grpc ./src/protocol-6/language-agent-v2/*.proto
    protoc -I ./src/protocol-6 --grpc_out=./src/grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./src/protocol-6/common/*.proto
    protoc -I ./src/protocol-6 --grpc_out=./src/grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./src/protocol-6/register/*.proto
    protoc -I ./src/protocol-6 --grpc_out=./src/grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./src/protocol-6/language-agent-v2/*.proto
  )

  AC_OUTPUT_COMMANDS(
   mv src/grpc/common/common.grpc.pb.cc src/grpc/common/common-grpc.pb.cc
   mv src/grpc/register/Register.grpc.pb.cc src/grpc/register/Register-grpc.pb.cc
   mv src/grpc/language-agent-v2/trace.grpc.pb.cc src/grpc/language-agent-v2/trace-grpc.pb.cc
   mv src/grpc/register/InstancePing.grpc.pb.cc src/grpc/register/InstancePing-grpc.pb.cc
   mv src/grpc/common/trace-common.grpc.pb.cc src/grpc/common/trace-common-grpc.pb.cc
  )

  dnl PHP_EVAL_LIBLINE($KYWALKING_LIBS, SKYWALKING_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, SKYWALKING_SHARED_LIBADD)
  PHP_ADD_BUILD_DIR($ext_builddir/src/grpc)
  PHP_ADD_INCLUDE("src/grpc")
  PHP_ADD_INCLUDE("src")
  PHP_SUBST(SKYWALKING_SHARED_LIBADD)

  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      src/greeter_client.cc \
      src/decode.c \
      src/encode.c \
      src/grpc/common/common-grpc.pb.cc \
      src/grpc/common/common.pb.cc \
      src/grpc/register/Register-grpc.pb.cc \
      src/grpc/register/Register.pb.cc \
      src/grpc/language-agent-v2/trace-grpc.pb.cc \
      src/grpc/language-agent-v2/trace.pb.cc \
      src/grpc/register/InstancePing-grpc.pb.cc \
      src/grpc/register/InstancePing.pb.cc \
      src/grpc/common/trace-common-grpc.pb.cc \
      src/grpc/common/trace-common.pb.cc \
  , $ext_shared,, -Wall -Werror -std=c++11)
fi
