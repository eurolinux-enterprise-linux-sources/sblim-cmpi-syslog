#
#
# Package spec for sblim-cmpi-syslog
#

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Summary: SBLIM Syslog providers
Name: sblim-cmpi-syslog
Version: 0.9.0
Release: 1
Group: Systems Management/Base
URL: http://www.sblim.org
License: EPL

BuildRequires: sblim-cmpi-base-devel >= 1.5.4

Requires: sblim-cmpi-base >= 1.5.4

Source0: http://prdownloads.sourceforge.net/sblim/%{name}-%{version}.tar.bz2 

%Description
Standards Based Linux Instrumentation File Syslog Providers

%Package test
Summary: SBLIM Syslog Instrumentation Testcase Files
Group: Systems Management/Base
Requires: %{name} = %{version}-%{release}
Requires: sblim-testsuite

%Description test
SBLIM Syslog Provider Testcase Files for the SBLIM Testsuite

%prep

%setup -q

%build

%configure TESTSUITEDIR=%{_datadir}/sblim-testsuite

make %{?_smp_mflags}

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

# remove unused libtool files
rm -f $RPM_BUILD_ROOT/%{_libdir}/*a
rm -f $RPM_BUILD_ROOT/%{_libdir}/cmpi/*a


%pre

%define SCHEMA %{_datadir}/%{name}/Syslog_Log.mof %{_datadir}/%{name}/Syslog_Service.mof  %{_datadir}/%{name}/Syslog_Configuration.mof
%define REGISTRATION %{_datadir}/%{name}/Syslog_Configuration.registration  %{_datadir}/%{name}/Syslog_Log.registration %{_datadir}/%{name}/Syslog_Service.registration

# If upgrading, deregister old version
if [ $1 -gt 1 ]
then
    %{_datadir}/%{name}/provider-register.sh -d \
        -r %{REGISTRATION} -m %{SCHEMA} > /dev/null
fi

%post
# Register Schema and Provider - this is higly provider specific

%{_datadir}/%{name}/provider-register.sh \
        -r %{REGISTRATION} -m %{SCHEMA} > /dev/null

/sbin/ldconfig

%preun
# Deregister only if not upgrading
if [ $1 -eq 0 ]
then
  %{_datadir}/%{name}/provider-register.sh -d \
        -r %{REGISTRATION} -m %{SCHEMA} > /dev/null
fi

%postun -p /sbin/ldconfig

%clean

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%docdir %{_datadir}/doc/%{name}-%{version}
%{_datadir}/%{name}
%{_datadir}/doc/%{name}-%{version}
%{_libdir}/*.so.*
%{_libdir}/cmpi/*.so

%files test
%defattr(-,root,root)
%{_datadir}/sblim-testsuite

%changelog
* Thu Jan 05 2006 Hien Nguyen <hien@us.ibm.com>
  - Add autoconf and spec.in

