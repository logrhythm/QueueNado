#!/bin/bash
set -e

PACKAGE=ZMQ
VERSION=1.0

PWD=`pwd`
CWD=$PWD/thirdParty
DISTDIR=$CWD/dist/g2log
PATH=$PATH:/usr/local/probe/bin:$PATH

rm -rf ~/rpmbuild
rpmdev-setuptree
cp thirdParty/packaging/$PACKAGE.spec ~/rpmbuild/SPECS
cp thirdParty/$PACKAGE-$VERSION.tar.gz ~/rpmbuild/SOURCES/$PACKAGE-$VERSION.tar.gz
cd ~/rpmbuild
rpmbuild -v -bb --target=x86_64 ~/rpmbuild/SPECS/$PACKAGE.spec

# Copy the artifacts to the local distribution directory
rm -rf $DISTDIR
mkdir -p $DISTDIR/include/
cp -r ~/rpmbuild/BUILD/$PACKAGE/src/*.h $DISTDIR/include
mkdir -p $DISTDIR/lib/
cp -r ~/rpmbuild/BUILD/$PACKAGE/*.so $DISTDIR/lib
