#!/bin/bash
set -e

PACKAGE=QueueNado


<<<<<<< HEAD
if [[ $# -ne 2 ]] ; then
    echo 'Usage:  sh buildRpm <BUILD_TYPE> <BUILD_NUMBER>'
    echo '        BUILD_TYPE is PRODUCTION or COVERAGE'
    exit 0
=======
if [[ $# -ne 1   ]] ; then
   echo 'Usage:  sh buildRpm <BUILD_TYPE>'
   echo '        BUILD_TYPE is PRODUCTION or DEBUG'
   exit 0
>>>>>>> main/master
fi

if [ "$1" = "PRODUCTION"   ] ; then
   BUILD_TYPE="-DUSE_LR_DEBUG=OFF"
elif  [ "$1" = "DEBUG"   ] ; then
   BUILD_TYPE="-DUSE_LR_DEBUG=ON"
else
   echo "<BUILD_TYPE> must be one of: PRODUCTION or DEBUG"
   exit 0
fi

<<<<<<< HEAD
BUILD="$2"


=======
>>>>>>> main/master
# As version number we use the commit number on HEAD 
# we do not bother with other branches for now
GIT_VERSION=`git rev-list --branches HEAD | wc -l`
VERSION="1.$GIT_VERSION"

echo "Building $PACKAGE --> version: $VERSION"

PWD=`pwd`
CWD=$PWD/$PACKAGE
DISTDIR=$CWD/dist/$PACKAGE
PATH=$PATH:/usr/local/probe/bin:$PATH

rm -rf ~/rpmbuild
rpmdev-setuptree
cp packaging/$PACKAGE.spec ~/rpmbuild/SPECS
rm -f $PACKAGE-$VERSION.tar.gz
tar czf $PACKAGE-$VERSION.tar.gz ./*
mkdir -p ~/rpmbuild/SOURCES
mv $PACKAGE-$VERSION.tar.gz ~/rpmbuild/SOURCES
cd ~/rpmbuild

<<<<<<< HEAD

rpmbuild -v -bb  --define="version ${VERSION}" --define="buildtype {$BUILD_TYPE}"  --define="buildnumber {$BUILD}" --target=x86_64 ~/rpmbuild/SPECS/$PACKAGE.spec
=======
rpmbuild -v -bb  --define="version ${VERSION}" --define="buildtype ${BUILD_TYPE}" --target=x86_64 ~/rpmbuild/SPECS/$PACKAGE.spec
>>>>>>> main/master
