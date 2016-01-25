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
PATH=/usr/local/probe/bin:$PATH
rm -f  CMakeCache.txt
cd 3rdparty
unzip -u gtest-1.7.0.zip
cd ../build



if [ "$BUILD_TYPE" == "-DUSE_DEBUG_COVERAGE=OFF" ]; then
   echo "buildtype: -DUSE_DEBUG_COVERAGE=OFF --> PRODUCTION for version: $VERSION"
   /usr/local/probe/bin/cmake -DVERSION=$VERSION -DCMAKE_CXX_COMPILER_ARG1:STRING=' -fPIC -Ofast -m64 -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 ' -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++ ..
elif [ "$BUILD_TYPE"== "-DUSE_DEBUG_COVERAGE=ON" ]; then
   echo "buildtype: -DUSE_DEBUG_COVERAGE=ON for version: $VERSION";
   /usr/local/probe/bin/cmake -DUSE_LR_DEBUG=ON -DVERSION=$VERSION  -DCMAKE_CXX_COMPILER_ARG1:STRING=' -Wall -Werror -g -gdwarf-2 -fprofile-arcs -ftest-coverage -O0 -fPIC -m64 -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 ' -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++ ..
else
   echo "unknown buildtype $BUILD_TYPE"
   exit 1
fi

make -j8
sudo ./UnitTestRunner