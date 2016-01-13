#!/bin/bash
LAUNCH_DIR=`pwd`

COVERAGE_DIR=coverage
# If you need to add a header to the code coverage please put it here
# example "myfile.h myotherfile.h"
HEADER_WHITELIST=

# black listing of files can also be done
# please see commented out execution of gcovr at the bottom
# example: 
#  SOURCE_BLACKLIST=".*ProbeTransmogrifier.cpp.*"

#clean up coverage dir
rm -rf $COVERAGE_DIR
mkdir -p $COVERAGE_DIR

cd 3rdparty
unzip -u gtest-1.7.0.zip
cd ..

rm -rf build
mkdir -p build
cd build


# As version number we use the commit number on HEAD 
# we do not bother with other branches for now
GIT_VERSION=`git rev-list --branches HEAD | wc -l`
VERSION="1.$GIT_VERSION"

PATH=/usr/local/probe/bin:$PATH
/usr/local/probe/bin/cmake -DUSE_LR_DEBUG=ON -DVERSION=$VERSION -DCMAKE_CXX_COMPILER_ARG1:STRING=' -Wall -Werror -g -gdwarf-2 -fprofile-arcs -ftest-coverage -O0 -fPIC -m64 -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 ' -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++ ..


make -j
sudo ./UnitTestRunner 
cd ..



PROJECT="QueueNado"
OBJECT_DIR="QueueNado.dir"
#copy source cpp files and profile files to the coverage dir
cp build/CMakeFiles/$OBJECT_DIR/src/* $COVERAGE_DIR
#convert the whitelist to a filter
FORMATTED_HEADER_LIST=
for header in $HEADER_WHITELIST 
do
    if [ -z $FORMATTED_HEADER_LIST ] ; then
        FORMATTED_HEADER_LIST=".*$header\$"
    else
        FORMATTED_HEADER_LIST="$FORMATTED_HEADER_LIST|.*$header\$"
    fi
done
cd $COVERAGE_DIR
if [ -z $FORMATTED_HEADER_LIST ] ; then
    FILTER=".*cpp$"
else
    FILTER=".*cpp$|$FORMATTED_HEADER_LIST"
fi

PATH=/usr/local/probe/bin:$PATH
# Uncomment/replace with the gcovr line at the bottom to enabled source blacklist
#gcovr -v --filter="$FILTER" --exclude="$SOURCE_BLACKLIST" --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage.html

gcovr -v --filter="$FILTER"  --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage.html
cd $LAUNCH_DIR