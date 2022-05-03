PHP_REQUIRE_CXX()
CXXFLAGS="$CXXFLAGS -Wall -std=c++11 -Wno-deprecated-register"

PHP_ARG_ENABLE([skywalking],
  [whether to enable skywalking support],
  [AS_HELP_STRING([--enable-skywalking],
    [Enable skywalking support])],
  [yes])

PHP_ARG_WITH([grpc_src],,
  [AS_HELP_STRING([[--with-grpc-src[=DIR]]],
    [Build with static gRPC library])],
  [no],
  [no])

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

  dnl shared grpc
  AC_MSG_CHECKING([for shared or static grpc])
  if test "$PHP_GRPC_SRC" == "no"; then
    AC_MSG_RESULT([shared])
    dnl shared grpc
    SEARCH_PATH="/usr/local /usr"
    SEARCH_GRPC_FOR="/include/grpc/grpc.h"
    SEARCH_PROTOBUF_FOR="/include/google/protobuf/message.h"

    AC_MSG_CHECKING([for grpc files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_GRPC_FOR; then
        GRPC_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done

    AC_MSG_CHECKING([for protobuf files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_PROTOBUF_FOR; then
        PROTOBUF_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done

    if test -z "$GRPC_DIR"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Please reinstall the grpc library])
    fi

    if test -z "$PROTOBUF_DIR"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Please reinstall the protobuf library])
    fi

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

    PHP_ADD_INCLUDE($GRPC_DIR/include)
    PHP_ADD_INCLUDE($PROTOBUF_DIR/include)
  else
    AC_MSG_RESULT([static $PHP_GRPC_SRC])

    dnl build grpc
    if ! test -f "$PHP_GRPC_SRC/cmake/build/libgrpc.a"; then
      CURRENT_PWD=`pwd`

      AC_MSG_NOTICE([switch to $PHP_GRPC_SRC])
      cd $PHP_GRPC_SRC
      rm -fr cmake/build
      mkdir -p cmake/build
      cd cmake/build
      cmake ../..
      make clean
      make -j$(nproc)

      AC_MSG_NOTICE([switch to $CURRENT_PWD])
      cd $CURRENT_PWD
    fi

    dnl search grpc .a
    SEARCH_GRPC_FOR="libgrpc.a libgpr.a libgrpc++.a libupb.a libaddress_sorting.a third_party/protobuf/libprotobuf.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/re2/libre2.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/status/libabsl_status.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/strings/libabsl_strings.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/strings/libabsl_strings_internal.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/strings/libabsl_str_format_internal.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/strings/libabsl_cord.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/numeric/libabsl_int128.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/types/libabsl_bad_optional_access.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/synchronization/libabsl_synchronization.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/synchronization/libabsl_graphcycles_internal.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/base/libabsl_base.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/base/libabsl_throw_delegate.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/base/libabsl_raw_logging_internal.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/base/libabsl_malloc_internal.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/base/libabsl_spinlock_wait.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/time/libabsl_time.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/time/libabsl_time_zone.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/debugging/libabsl_symbolize.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/debugging/libabsl_stacktrace.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/abseil-cpp/absl/debugging/libabsl_debugging_internal.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/cares/cares/lib/libcares.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/boringssl-with-bazel/libssl.a"
    SEARCH_GRPC_FOR="$SEARCH_GRPC_FOR third_party/boringssl-with-bazel/libcrypto.a"
    AC_MSG_CHECKING([for grpc files in $PHP_GRPC_SRC path])
    for i in $SEARCH_GRPC_FOR ; do
      target=$PHP_GRPC_SRC/cmake/build/$i
      if test -r $target; then
        AC_MSG_RESULT(found in $target)
      else
        AC_MSG_ERROR([not found $target])
      fi
    done

    dnl ldflags
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/libgrpc++.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/libgrpc.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/libgpr.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/libupb.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/libaddress_sorting.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/protobuf/libprotobuf.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/re2/libre2.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/status/libabsl_status.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/strings/libabsl_strings.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/strings/libabsl_strings_internal.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/strings/libabsl_str_format_internal.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/strings/libabsl_cord.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/numeric/libabsl_int128.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/types/libabsl_bad_optional_access.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/synchronization/libabsl_synchronization.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/synchronization/libabsl_graphcycles_internal.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/base/libabsl_base.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/base/libabsl_throw_delegate.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/base/libabsl_raw_logging_internal.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/base/libabsl_malloc_internal.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/base/libabsl_spinlock_wait.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/time/libabsl_time.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/time/libabsl_time_zone.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/debugging/libabsl_symbolize.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/debugging/libabsl_stacktrace.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/abseil-cpp/absl/debugging/libabsl_debugging_internal.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/cares/cares/lib/libcares.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/boringssl-with-bazel/libssl.a"
    SKYWALKING_EXTRA_LDFLAGS="$SKYWALKING_EXTRA_LDFLAGS $PHP_GRPC_SRC/cmake/build/third_party/boringssl-with-bazel/libcrypto.a"

    PHP_ADD_INCLUDE($PHP_GRPC_SRC/include)
    PHP_ADD_INCLUDE($PHP_GRPC_SRC/third_party/protobuf/src)

    EXTRA_LDFLAGS="$EXTRA_LDFLAGS $SKYWALKING_EXTRA_LDFLAGS"
  fi

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

  if test "$PHP_GRPC_SRC" == "no"; then
    AC_PATH_PROG(PROTOC, protoc, no)
  else
    AC_PATH_PROG(PROTOC, protoc, no, $PHP_GRPC_SRC/cmake/build/third_party/protobuf)
  fi
  if ! test -x "$PROTOC"; then
    AC_MSG_ERROR([protoc command missing, please reinstall the protobuf distribution])
  fi

  if test "$PHP_GRPC_SRC" == "no"; then
    AC_PATH_PROG(GRPC_CPP_PLUGIN, grpc_cpp_plugin, no)
  else
    AC_PATH_PROG(GRPC_CPP_PLUGIN, grpc_cpp_plugin, no, $PHP_GRPC_SRC/cmake/build)
  fi
  if ! test -x "$GRPC_CPP_PLUGIN"; then
    AC_MSG_ERROR([grpc_cpp_plugin command missing, please reinstall the grpc distribution])
  fi

  mkdir -p src/network/v3
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/common/Common.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/language-agent/*.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/profile/*.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/management/*.proto
  $PROTOC -I src/protocol/v3 --grpc_out=src/network/v3 --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN --cpp_out=src/network/v3 src/protocol/v3/logging/*.proto
  find src -name "*.grpc.pb.cc" | while read id; do mv $id ${id/.grpc/_grpc}; done

  dnl shared grpc
  if test "$PHP_GRPC_SRC" == "no"; then
    PROTOBUF_LIBDIR=$PROTOBUF_DIR/${PROTOBUF_LIB_SUBDIR-lib}
    PHP_ADD_LIBPATH($PROTOBUF_LIBDIR)
    PHP_ADD_LIBRARY(protobuf,,SKYWALKING_SHARED_LIBADD)
    AC_DEFINE(HAVE_PROTOBUFLIB,1,[ ])
  fi

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
      src/sky_core_span_log.cc \
      src/sky_execute.cc \
      src/sky_log.cc \
      src/sky_module.cc \
      src/sky_plugin_mysqli.cc \
      src/sky_pdo.cc \
      src/sky_plugin_curl.cc \
      src/sky_plugin_error.cc \
      src/sky_plugin_grpc.cc \
      src/sky_plugin_hyperf_guzzle.cc \
      src/sky_plugin_predis.cc \
      src/sky_plugin_rabbit_mq.cc \
      src/sky_plugin_redis.cc \
      src/sky_plugin_memcached.cc \
      src/sky_plugin_yar.cc \
      src/sky_plugin_swoole_curl.cc \
      src/sky_rate_limit.cc \
      src/sky_shm.cc \
      src/sky_utils.cc \
      src/span.cc \
      src/tag.cc \
      src/logging/logging_data_body.cc \
      src/logging/logging_tag.cc \
      src/logging/logging_trace_context.cc \
      src/logging/logging_data.cc \
      src/sky_plugin_logging.cc \
      src/logging/logging_hander_yii.cc \
      src/logging/logging_hander_thinkphp.cc \
      src/logging/logging_hander_internal.cc \
      src/logging/logging_manager.cc \
      src/logging/logging_common.cc \
      src/network/v3/common/Common_grpc.pb.cc \
      src/network/v3/common/Common.pb.cc \
      src/network/v3/language-agent/Tracing.pb.cc \
      src/network/v3/language-agent/Tracing_grpc.pb.cc \
      src/network/v3/management/Management_grpc.pb.cc \
      src/network/v3/management/Management.pb.cc \
      src/network/v3/logging/Logging_grpc.pb.cc \
      src/network/v3/logging/Logging.pb.cc \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1, cxx)
fi

if test -r $phpincludedir/ext/mysqli/mysqli_mysqlnd.h; then
    AC_DEFINE([MYSQLI_USE_MYSQLND], 1, [Whether mysqlnd is enabled])
fi
