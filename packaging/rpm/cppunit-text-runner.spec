Name:           cppunit-text-runner
Version:        0.1
Release:        1%{?dist}
License:        GPL3
Summary:        Text runner for cppunit loading tests from a shared lib
Group:          Development/Tools
Packager:       Lukasz Janyst <ljanyst@cern.ch>
URL:            https://github.com/ljanyst/cppunit-text-runner
Source0:        %{name}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires:       cppunit
BuildRequires:  cppunit-devel
BuildRequires:  cmake

%description
Text runner for cppunit loading tests form a shared lib

#-------------------------------------------------------------------------------
# Build things
#-------------------------------------------------------------------------------
%prep
%setup -c -n %{name}

%build
cd %{name}
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo ../

#-------------------------------------------------------------------------------
# Install section
#-------------------------------------------------------------------------------
%install
cd %{name}
cd build
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------
%files
%defattr(-,root,root,-)
%{_bindir}/cppunit-text-runner
%doc %{name}/COPYING

#-------------------------------------------------------------------------------
# Changelog
#-------------------------------------------------------------------------------
%changelog
* Wed Oct 16 2013 Lukasz Janyst <ljanyst@cern.ch>
- initial package (0.1)
