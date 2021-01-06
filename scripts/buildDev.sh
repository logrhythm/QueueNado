#!/bin/bash
set -e
if [[ $# -ne 1 ]] ; then
    echo 'Usage:  sh builDev <BUILD_TYPE>'
    echo '        BUILD_TYPE is PRODUCTION or COVERAGE'
    exit 0
fi
 
if [ "$1" = "PRODUCTION" ] ; then
   BUILD_TYPE="-DUSE_DEBUG_COVERAGE=OFF"
elif  [ "$1" = "COVERAGE" ] ; then
   BUILD_TYPE="-DUSE_DEBUG_COVERAGE=ON"
else
   echo "<BUILD_TYPE> must be one of: PRODUCTION or COVERAGE"
   exit 0
fi


GIT_VERSION=`git rev-list HEAD --count`
VERSION="1.$GIT_VERSION"

rm -rf build
mkdir -p  build
rm -f  CMakeCache.txt
sh scripts/getLibraries
cd build


if [ "$BUILD_TYPE" == "-DUSE_DEBUG_COVERAGE=OFF" ]; then
   echo "buildtype: -DUSE_DEBUG_COVERAGE=OFF --> PRODUCTION for version: $VERSION"
  /usr/bin/cmake -DUSE_LR_DEBUG=ON -DVERSION:STRING=$VERSION \
      -DCMAKE_CXX_COMPILER_ARG1:STRING=' -std=c++14 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -Wall -fPIC -Ofast -m64 -isystem/usr/local/probe/include -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib ' \
      -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_SHARED_LIBS:BOOL=ON ..
elif [ "$BUILD_TYPE" == "-DUSE_DEBUG_COVERAGE=ON" ]; then
   echo "buildtype: -DUSE_DEBUG_COVERAGE=ON for version: $VERSION";
  /usr/bin/cmake -DVERSION:STRING=$VERSION \
      -DCMAKE_CXX_COMPILER_ARG1:STRING=' -std=c++14 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -Wall -g -gdwarf-2 -O0 -fPIC -m64 -isystem/usr/local/gcc/include -isystem/usr/local/probe/include -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/gcc/lib64 ' ..
else
   echo "unknown buildtype $BUILD_TYPE"
   exit 1
fi

make -j8
sudo ./UnitTestRunner
