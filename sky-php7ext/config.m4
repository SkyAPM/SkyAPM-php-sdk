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
[  --enable-skywalking           Enable skywalking support])

if test "$PHP_SKYWALKING" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-skywalking -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/skywalking.h"  # you most likely want to change this
  dnl if test -r $PHP_SKYWALKING/$SEARCH_FOR; then # path given as parameter
  dnl   SKYWALKING_DIR=$PHP_SKYWALKING
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for skywalking files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SKYWALKING_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SKYWALKING_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the skywalking distribution])
  dnl fi

  dnl # --with-skywalking -> add include path
  dnl PHP_ADD_INCLUDE($SKYWALKING_DIR/include)

  dnl # --with-skywalking -> check for lib and symbol presence
  dnl LIBNAME=skywalking # you may want to change this
  dnl LIBSYMBOL=skywalking # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SKYWALKING_DIR/$PHP_LIBDIR, SKYWALKING_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SKYWALKINGLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong skywalking lib version or lib not found])
  dnl ],[
  dnl   -L$SKYWALKING_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SKYWALKING_SHARED_LIBADD)
  PHP_REQUIRE_CXX()

  KYWALKING_LIBS=" -L/usr/local/lib "
  KYWALKING_LIBS+=`pkg-config --libs protobuf grpc++ grpc`
  KYWALKING_LIBS+="-Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl"

  AC_OUTPUT_COMMANDS(
    protoc -I ./protos --cpp_out=./grpc ./protos/DiscoveryService.proto
    protoc -I ./protos --cpp_out=./grpc ./protos/ApplicationRegisterService.proto
    protoc -I ./protos --cpp_out=./grpc ./protos/TraceSegmentService.proto
  )
  AC_OUTPUT_COMMANDS(
    protoc -I ./protos --grpc_out=./grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./protos/DiscoveryService.proto 
    protoc -I ./protos --grpc_out=./grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./protos/ApplicationRegisterService.proto 
    protoc -I ./protos --grpc_out=./grpc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./protos/TraceSegmentService.proto 
  )
  AC_OUTPUT_COMMANDS(
    mv grpc/DiscoveryService.grpc.pb.cc grpc/DiscoveryService-grpc.pb.cc
    mv grpc/ApplicationRegisterService.grpc.pb.cc grpc/ApplicationRegisterService-grpc.pb.cc
    mv grpc/TraceSegmentService.grpc.pb.cc grpc/TraceSegmentService-grpc.pb.cc
  )

  PHP_EVAL_LIBLINE($KYWALKING_LIBS, SKYWALKING_SHARED_LIBADD)


  PHP_NEW_EXTENSION(skywalking, \
      skywalking.c \
      grpc/greeter_client.cc \
      grpc/DiscoveryService.pb.cc \
      grpc/DiscoveryService-grpc.pb.cc \
      grpc/ApplicationRegisterService.pb.cc \
      grpc/ApplicationRegisterService-grpc.pb.cc \
      grpc/TraceSegmentService.pb.cc \
      grpc/TraceSegmentService-grpc.pb.cc \
  , $ext_shared,, -std=c++11 -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)

  PHP_ADD_BUILD_DIR($ext_builddir/grpc)

  PHP_SUBST(SKYWALKING_SHARED_LIBADD)
fi
