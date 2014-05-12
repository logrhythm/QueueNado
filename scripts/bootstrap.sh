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
/root/rpmbuild/BUILD/FileIO/UnitTestRunner
cp /root/rpmbuild/RPMS/*/*.rpm rpms
cd ..

cd Death
sh scripts/buildRpm.sh >> /tmp/bootstrap.out 2>&1
$RPMCOMMAND $RPMARGS /root/rpmbuild/RPMS/x86_64/*.rpm >> /tmp/bootstrap.out 2>&1
/root/rpmbuild/BUILD/Death/UnitTestRunner
cp /root/rpmbuild/RPMS/*/*.rpm rpms
cd ..

cd ZMQ
sh scripts/buildRpm.sh >> /tmp/bootstrap.out 2>&1
$RPMCOMMAND $RPMARGS /root/rpmbuild/RPMS/x86_64/*.rpm >> /tmp/bootstrap.out 2>&1
/root/rpmbuild/BUILD/Death/UnitTestRunner
cd ..
cp /root/rpmbuild/RPMS/*/*.rpm rpms


