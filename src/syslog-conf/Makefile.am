# SBLIM Include Directory
sblimincdir = $(includedir)/sblim

# Start from an empty extra distribution file list
EXTRA_DIST=
doc_DATA=

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script).
providerdir = @PROVIDERDIR@

provider_LTLIBRARIES = libSyslog_Configuration.la \
		libSyslog_Setting.la \
		libSyslog_SettingContext.la

libSyslog_Configuration_la_SOURCES = Syslog_Configuration.c
libSyslog_Configuration_la_LIBADD = -lSyslog_ConfUtils -lsyslogconfutil -lsyslogsettingparse
libSyslog_Configuration_la_LDFLAGS = -avoid-version

libSyslog_Setting_la_SOURCES = Syslog_Setting.c
libSyslog_Setting_la_LIBADD = -lSyslog_ConfUtils -lsyslogconfutil -lsyslogsettingparse
libSyslog_Setting_la_LDFLAGS = -avoid-version

libSyslog_SettingContext_la_SOURCES = Syslog_SettingContext.c
libSyslog_SettingContext_la_LIBADD = -lSyslog_ConfUtils -lsyslogconfutil -lsyslogsettingparse
libSyslog_SettingContext_la_LDFLAGS = -avoid-version

lib_LTLIBRARIES = libSyslog_ConfUtils.la libsyslogconfutil.la libsyslogsettingparse.la
libSyslog_ConfUtils_la_SOURCES= Syslog_ConfUtils.c
#libSyslog_ConfUtils_la_LIBADD = -lsyslogconfutil -lsyslogsettingparse
libSyslog_ConfUtils_la_LDFLAGS = -Wc,-nostartfiles
libsyslogconfutil_la_SOURCES = util/syslogconfutil.c
libsyslogconfutil_la_LDFLAGS = -Wc,-nostartfiles
libsyslogsettingparse_la_SOURCES = util/syslogsettingparse.c
libsyslogsettingparse_la_LDFLAGS = -Wc,-nostartfiles

noinst_HEADERS = Syslog_ConfUtils.h util/syslogconfutil.h

EXTRA_DIST+=README

#
# Mof file
#
SCHEMAS=mof/Syslog_Configuration.mof
REGISTRATIONS=mof/Syslog_Configuration.registration

pkgdata_DATA=$(SCHEMAS) $(REGISTRATIONS)
pkgdata_SCRIPTS=provider-register.sh

EXTRA_DIST+=mof $(pkgdata_SCRIPTS)
# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	cp /etc/syslog.conf /etc/syslogtest.conf
	sh provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)
preuninstall:
	rm -f /etc/syslogtest.conf
	sh provider-register.sh -d -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)

#
# Automake instructions for ./test subdir
#
EXTRA_DIST+=test
if TESTSUITE
testsuitedir=@TESTSUITEDIR@
testsuitesystemdir=@TESTSUITEDIR@/system/linux
testsuitecimdir=@TESTSUITEDIR@/cim

testsuite_SCRIPTS=test/test-cmpi-syslog-conf.sh \
		test/runtest_wbemcli.sh \
		test/runtest_pegasus.sh

testsuitesystem_DATA=test/system/linux/Syslog_Configuration.system \
		test/system/linux/Syslog_SettingContext.system \
		test/system/linux/Syslog_Setting.system

testsuitesystem_PROGRAMS=setting
setting_SOURCES=test/system/linux/setting.c

testsuitecim_DATA=test/cim/Syslog_Configuration.cim \
		test/cim/Syslog_Setting.cim \
		test/cim/Syslog_SettingContext.cim

runtest: install
	cd $(DESTDIR)$(TESTSUITEDIR) && ./test-cmpi-syslog-conf.sh

doc_DATA+=test/readme.test-syslog-conf
endif

