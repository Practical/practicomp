#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([The Practical Programming Language Compiler], [0.0], [shachar@shemesh.biz])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_SRCDIR([examples/true.pr])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SUBDIRS([external/practical-sa])
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT

m4_include([m4/ax_cxx_compile_stdcxx.m4])

LLVM_VERSION=7

AC_LANG(C++)
# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG
AC_CHECK_PROG(LLVM_CONFIG, [llvm-config-$LLVM_VERSION], [yes], [no])
test "yes" == "$LLVM_CONFIG" || AC_MSG_ERROR([Couldn't find llvm-config-$LLVM_VERSION: is LLVM installed?])

# Checks for libraries.
AC_SUBST([LLVM_CXXFLAGS], [`llvm-config-$LLVM_VERSION --cxxflags | tr ' ' '\n' | grep -v -- ^-std= | tr '\n' ' '`])
AC_SUBST([LLVM_LDFLAGS], `llvm-config-$LLVM_VERSION --ldflags`)
AC_SUBST([LLVM_LIBS], `llvm-config-$LLVM_VERSION --libs`)

# Checks for header files.
AC_CHECK_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AX_CXX_COMPILE_STDCXX(17, [noext], [mandatory])

# Checks for library functions.

AC_DEFINE_UNQUOTED([BUILD_TRIPLET], ["$build"], [Platform triplet on which the compiler was built])
AC_DEFINE_UNQUOTED([HOST_TRIPLET], ["$host"], [Platform triplet on which the compiler will run])
AC_DEFINE_UNQUOTED([TARGET_TRIPLET], ["$target"], [Platform triplet for which the compiler will produce code])
AC_DEFINE_UNQUOTED([TARGET_CPU], ["$target_cpu"], [CPU for which the compiler will produce code])
AC_DEFINE_UNQUOTED([TARGET_OS], ["$target_os"], [Operating system for which the compiler will produce code])

AC_DEFINE([PRACTICAL_SOURCE_FILE_EXTENSION], [".pr"], [Expected extension for Practical source files])
AC_DEFINE([OBJECT_FILE_EXTENSION], [".o"], [Output extension of object files])

AC_CONFIG_FILES([Makefile external/Makefile compiler/Makefile])
AC_OUTPUT
