#!/bin/sh
if [ "$1" == "" ]; then
   RPMCOMMAND=rpm
else
   RPMCOMMAND=echo
fi
RPMARGS="-Uvh --replacepkgs --replacefiles --force  --nodeps "
mkdir -p rpms
touch /tmp/bootstrap.out 
set -e
set -x

cd FileIO
sh scripts/buildRpm.sh >> /tmp/bootstrap.out 2>&1
$RPMCOMMAND $RPMARGS /root/rpmbuild/RPMS/x86_64/*.rpm >> /tmp/bootstrap.out 2>&1
cd ..
cd Death
sh scripts/buildRpm.sh >> /tmp/bootstrap.out 2>&1
$RPMCOMMAND $RPMARGS /root/rpmbuild/RPMS/x86_64/*.rpm >> /tmp/bootstrap.out 2>&1
cd ..
cp /root/rpmbuild/RPMS/*/*.rpm rpms
(cd Death/3rdparty ; tar cf - gtest-1.7.0 ) | (cd ZMQ/3rdparty ; tar xf - )
cd ZMQ
sh scripts/buildRpm.sh >> /tmp/bootstrap.out 2>&1
$RPMCOMMAND $RPMARGS /root/rpmbuild/RPMS/x86_64/*.rpm >> /tmp/bootstrap.out 2>&1
cd ..
cp /root/rpmbuild/RPMS/*/*.rpm rpms


