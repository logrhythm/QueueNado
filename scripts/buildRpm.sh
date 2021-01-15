#!/bin/bash
set -e

PACKAGE=QueueNado


if [[ $# -ne 2 ]] ; then
    echo 'Usage:  sh buildRpm <BUILD_NUMBER> <BUILD_TYPE>'
    echo '        BUILD_TYPE is PRODUCTION or COVERAGE'
    exit 0
fi

if [ "$2" = "PRODUCTION" ] ; then
   BUILD_TYPE="-DUSE_LR_DEBUG=OFF"
elif [ "$2" = "DEBUG" ] ; then
   BUILD_TYPE="-DUSE_LR_DEBUG=ON"
else
   echo "<BUILD_TYPE> must be one of: PRODUCTION or DEBUG"
   exit 0
fi

BUILD="$1"


# As version number we use the commit number on HEAD 
# we do not bother with other branches for now
GIT_VERSION=`git rev-list --branches HEAD | wc -l`
VERSION="1.$GIT_VERSION"

echo "Building $PACKAGE --> version: $VERSION"

PWD=`pwd`
CWD=$PWD/$PACKAGE

sudo rm -rf ~/rpmbuild
rpmdev-setuptree
cp packaging/$PACKAGE.spec ~/rpmbuild/SPECS
rm -f $PACKAGE-$VERSION.tar.gz
tar czf $PACKAGE-$VERSION.tar.gz ./*
mkdir -p ~/rpmbuild/SOURCES
mv $PACKAGE-$VERSION.tar.gz ~/rpmbuild/SOURCES
cd ~/rpmbuild

rpmbuild -v -bb --define="version ${VERSION}" --define="buildtype {$BUILD_TYPE}" --define="buildnumber {$BUILD}" --target=x86_64 ~/rpmbuild/SPECS/$PACKAGE.spec
