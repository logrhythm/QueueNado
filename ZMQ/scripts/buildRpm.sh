#!/bin/bash
set -e

PACKAGE=ZMQ
VERSION=1.0

PWD=`pwd`
CWD=$PWD/ZMQ
DISTDIR=$CWD/dist/ZMQ
PATH=$PATH:/usr/local/probe/bin:$PATH

rm -rf ~/rpmbuild
rpmdev-setuptree
cp packaging/$PACKAGE.spec ~/rpmbuild/SPECS
rm -f $PACKAGE-$VERSION.tar.gz
tar czf $PACKAGE-$VERSION.tar.gz ./*
cp $PACKAGE-$VERSION.tar.gz ~/rpmbuild/SOURCES
cd ~/rpmbuild
rpmbuild -v -bb --target=x86_64 ~/rpmbuild/SPECS/$PACKAGE.spec

# Copy the artifacts to the local distribution directory
rm -rf $DISTDIR
mkdir -p $DISTDIR/include/
cp -r ~/rpmbuild/BUILD/$PACKAGE/src/*.h $DISTDIR/include
mkdir -p $DISTDIR/lib/
cp -r ~/rpmbuild/BUILD/$PACKAGE/*.so $DISTDIR/lib
