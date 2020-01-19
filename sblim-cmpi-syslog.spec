%global provider_dir %{_libdir}/cmpi

Summary:        SBLIM syslog instrumentation
Name:           sblim-cmpi-syslog
Version:        0.9.0
Release:        2%{?dist}
License:        EPL
Group:          Applications/System
URL:            http://sourceforge.net/projects/sblim/
# The source for this package was pulled from upstream's vcs.  Use the
# following commands to generate the tarball:
#  cvs -z3 -d:pserver:anonymous@sblim.cvs.sourceforge.net:/cvsroot/sblim co -P cmpi-syslog
#  mv cmpi-syslog sblim-cmpi-syslog-0.8.0
#  tar -cJvf sblim-cmpi-syslog-0.8.0.tar.xz sblim-cmpi-syslog-0.8.0
Source0:        %{name}-%{version}.tar.bz2

# use Pegasus' root/interop instead of root/PG_Interop
Patch0:         sblim-cmpi-syslog-0.9.0-pegasus-interop.patch

BuildRequires:  sblim-cmpi-devel
BuildRequires:  sblim-cmpi-base-devel >= 1.5.4
BuildRequires:  libtool
Requires:       sblim-cmpi-base >= 1.5.4 cim-server
Requires:       /etc/ld.so.conf.d
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Standards Based Linux Instrumentation Syslog Providers

%package devel
# ^- currently a placeholder - no devel files shipped
Summary:        SBLIM Syslog Instrumentation Header Development Files
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description devel
SBLIM Base Syslog Development Package

%package test
Summary:        SBLIM Syslog Instrumentation Testcases
Group:          Applications/System
Requires:       %{name} = %{version}-%{release}
Requires:       sblim-testsuite

%description test
SBLIM Base Syslog Testcase Files for SBLIM Testsuite

%prep
%setup -q
%patch0 -p1 -b .interop
# removing COPYING, because it's misleading
rm -f COPYING
# ./autoconfiscate.sh

%build
%ifarch s390 s390x ppc ppc64
export CFLAGS="$RPM_OPT_FLAGS -fsigned-char"
%else
export CFLAGS="$RPM_OPT_FLAGS" 
%endif
%configure \
        TESTSUITEDIR=%{_datadir}/sblim-testsuite \
        CIMSERVER=sfcb \
        PROVIDERDIR=%{provider_dir} \
        SYSLOG=rsyslog
sed -i 's|^hardcode_libdir_flag_spec=.*|hardcode_libdir_flag_spec=""|g' libtool
sed -i 's|^runpath_var=LD_RUN_PATH|runpath_var=DIE_RPATH_DIE|g' libtool
make

%install
make install DESTDIR=$RPM_BUILD_ROOT
# remove unused libtool files
rm -f $RPM_BUILD_ROOT/%{_libdir}/*a
rm -f $RPM_BUILD_ROOT/%{provider_dir}/*a
# shared libraries
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/ld.so.conf.d
echo "%{_libdir}/cmpi" > $RPM_BUILD_ROOT/%{_sysconfdir}/ld.so.conf.d/%{name}-%{_arch}.conf
# move libraries to provider dir
mv $RPM_BUILD_ROOT/%{_libdir}/lib[Ss]yslog*.so* $RPM_BUILD_ROOT/%{provider_dir}
# add shebang to the scripts
sed -i -e '1i#!/bin/sh' $RPM_BUILD_ROOT/%{_bindir}/syslog-service.sh \
$RPM_BUILD_ROOT/%{_datadir}/sblim-testsuite/runtest_pegasus.sh \
$RPM_BUILD_ROOT/%{_datadir}/sblim-testsuite/runtest_wbemcli.sh \
$RPM_BUILD_ROOT/%{_datadir}/sblim-testsuite/system/linux/logrecord.sh \
$RPM_BUILD_ROOT/%{_datadir}/sblim-testsuite/system/linux/msglogtest.sh \
$RPM_BUILD_ROOT/%{_datadir}/sblim-testsuite/system/linux/messagelog.sh

%files
%{_bindir}/syslog-service.sh
%{provider_dir}/lib[Ss]yslog*.so*
%{_datadir}/%{name}
%docdir %{_datadir}/doc/%{name}-%{version}
%{_datadir}/doc/%{name}-%{version}
%config(noreplace) %{_sysconfdir}/ld.so.conf.d/%{name}-%{_arch}.conf

%files test
%{_datadir}/sblim-testsuite/runtest*
%{_datadir}/sblim-testsuite/test-cmpi-syslog*
%{_datadir}/sblim-testsuite/cim/Syslog*
%{_datadir}/sblim-testsuite/system/linux/Syslog*
%{_datadir}/sblim-testsuite/system/linux/logrecord.sh
%{_datadir}/sblim-testsuite/system/linux/messagelog.sh
%{_datadir}/sblim-testsuite/system/linux/msglogtest.sh
%{_datadir}/sblim-testsuite/system/linux/setting

%global SYSLOG_SCHEMA %{_datadir}/sblim-cmpi-syslog/Syslog_Log.mof %{_datadir}/sblim-cmpi-syslog/Syslog_Service.mof  %{_datadir}/sblim-cmpi-syslog/Syslog_Configuration.mof
%global SYSLOG_REGISTRATION %{_datadir}/sblim-cmpi-syslog/Syslog_Configuration.registration  %{_datadir}/sblim-cmpi-syslog/Syslog_Log.registration %{_datadir}/sblim-cmpi-syslog/Syslog_Service.registration

%pre
function unregister()
{
  %{_datadir}/%{name}/provider-register.sh -d \
        $1 \
        -m %{SYSLOG_SCHEMA} \
        -r %{SYSLOG_REGISTRATION} > /dev/null 2>&1 || :;
  # don't let registration failure when server not running fail upgrade!
}

# If upgrading, deregister old version
if [ $1 -gt 1 ]
then
        unregistered=no
        if [ -e /usr/sbin/cimserver ]; then
           unregister "-t pegasus";
           unregistered=yes
        fi

        if [ -e /usr/sbin/sfcbd ]; then
           unregister "-t sfcb";
           unregistered=yes
        fi

        if [ "$unregistered" != yes ]; then
           unregister
        fi
fi

%post
function register()
{
  # The follwoing script will handle the registration for various CIMOMs.
  %{_datadir}/%{name}/provider-register.sh \
        $1 \
        -m %{SYSLOG_SCHEMA} \
        -r %{SYSLOG_REGISTRATION} > /dev/null 2>&1 || :;
  # don't let registration failure when server not running fail install!
}

/sbin/ldconfig
if [ $1 -ge 1 ]
then
        registered=no
        if [ -e /usr/sbin/cimserver ]; then
          register "-t pegasus";
          registered=yes
        fi

        if [ -e /usr/sbin/sfcbd ]; then
          register "-t sfcb";
          registered=yes
        fi

        if [ "$registered" != yes ]; then
          register
        fi
fi

%preun
function unregister()
{
  %{_datadir}/%{name}/provider-register.sh -d \
        $1 \
        -m %{SYSLOG_SCHEMA} \
        -r %{SYSLOG_REGISTRATION} > /dev/null 2>&1 || :;
  # don't let registration failure when server not running fail erase!
}

if [ $1 -eq 0 ]
then
        unregistered=no
        if [ -e /usr/sbin/cimserver ]; then
          unregister "-t pegasus";
          unregistered=yes
        fi

        if [ -e /usr/sbin/sfcbd ]; then
          unregister "-t sfcb";
          unregistered=yes
        fi

        if [ "$unregistered" != yes ]; then
          unregister
        fi
fi

%postun -p /sbin/ldconfig

%changelog
* Wed Aug 14 2013 Vitezslav Crhonek <vcrhonek@redhat.com> - 0.9.0-2
- Use Pegasus root/interop instead of root/PG_Interop

* Fri Jul 26 2013 Tomas Bzatek <tbzatek@redhat.com> - 0.9.0-1
- Update to sblim-cmpi-syslog-0.9.0

* Mon Jul 01 2013 Tomas Bzatek <tbzatek@redhat.com> - 0.8.0-10.20120315cvs
- Add support for rsyslog $IncludeConfig directive (#971807)
- Add support for filter definitions (#971807)
- Various rsyslog compatibility fixes (#971807)

* Thu May 30 2013 Tomas Bzatek <tbzatek@redhat.com> - 0.8.0-9.20120315cvs
- Update scriptlets to register with both sfcbd and pegasus

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.0-8.20120315cvs
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Mon Nov 19 2012 Vitezslav Crhonek <vcrhonek@redhat.com> - 0.8.0-7.20120315cvs
- Set rsyslogd as default

* Thu Sep 06 2012 Vitezslav Crhonek <vcrhonek@redhat.com> - 0.8.0-6.20120315cvs
- Fix issues found by fedora-review utility in the spec file

* Sat Jul 21 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.0-5.20120315cvs
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Mar 15 2012 Vitezslav Crhonek <vcrhonek@redhat.com> - 0.8.0-4.20120315cvs
- Use latest upstream CVS (because of rsyslog support)
- Build against sblim-cmpi-devel and instead of tog-pegasus-devel
- Fix rsyslog support to work with systemd

* Sat Jan 14 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Wed Feb 09 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.8.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Thu Nov  4 2010 Vitezslav Crhonek <vcrhonek@redhat.com> - 0.8.0-1
- Update to sblim-cmpi-syslog-0.8.0
- Remove CIMOM dependencies

* Fri Oct 23 2009 Vitezslav Crhonek <vcrhonek@redhat.com> - 0.7.11-1
- Initial support
