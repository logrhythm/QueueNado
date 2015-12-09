Name:          QueueNado
Version:       1.0
Release:       1%{?dist}
Summary:       An implemnetation of Queue patterns for C++
Group:         Development/Tools
License:       MIT
BuildRequires: probecmake >= 2.8, probezlib, g3logrotate, StopWatch
Requires: g3log, DeathKnell, czmq >= 2.0.3
ExclusiveArch: x86_64

%description

%prep
cd ~/rpmbuild/BUILD
rm -rf %{name}
mkdir %{name}
cd %{name}
tar xzf ~/rpmbuild/SOURCES/%{name}-%{version}.tar.gz
if [ $? -ne 0 ]; then
   exit $?
fi

%build
# SKIP_BUILD_RPATH, CMAKE_SKIP_BUILD_RPATH, 
cd %{name}/
PATH=/usr/local/probe/bin:$PATH
rm -f  CMakeCache.txt
cd 3rdparty
unzip -u gtest-1.7.0.zip
cd ..
/usr/local/probe/bin/cmake -DCMAKE_CXX_COMPILER_ARG1:STRING=' -fPIC -Ofast -m64 -Wl,-rpath,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 ' -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++
make -j8
#sudo ./UnitTestRunner
mkdir -p $RPM_BUILD_ROOT/usr/local/probe/lib
cp *.so $RPM_BUILD_ROOT/usr/local/probe/lib
rm $RPM_BUILD_ROOT/usr/local/probe/lib/libgtest_170_lib.so
mkdir -p $RPM_BUILD_ROOT/usr/local/probe/include
cp src/*.h $RPM_BUILD_ROOT/usr/local/probe/include
rm -f $RPM_BUILD_ROOT/usr/local/probe/include/QueueNadoMacros.h

%post

%preun

%postun

%files
%defattr(-,dpi,dpi,-)
/usr/local/probe/lib
/usr/local/probe/include
