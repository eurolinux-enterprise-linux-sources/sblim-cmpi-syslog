# SBLIM Include Directory
sblimincdir = $(includedir)/sblim

# Start from an empty extra distribution file list
EXTRA_DIST=
doc_DATA=

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script).
providerdir = @PROVIDERDIR@

provider_LTLIBRARIES = libSyslog_Service.la \
		libSyslog_ServiceProcess.la

libSyslog_Service_la_SOURCES = Syslog_Service.c
libSyslog_Service_la_LIBADD = -lSyslog_ServiceUtils -lsyslogserviceutil
libSyslog_Service_la_LDFLAGS = -avoid-version

libSyslog_ServiceProcess_la_SOURCES = Syslog_ServiceProcess.c
libSyslog_ServiceProcess_la_CFLAGS = -I$(sblimincdir)
libSyslog_ServiceProcess_la_LIBADD = -lSyslog_ServiceUtils -lsyslogserviceutil -lcmpiOSBase_Common
libSyslog_ServiceProcess_la_LDFLAGS = -avoid-version

lib_LTLIBRARIES = libSyslog_ServiceUtils.la libsyslogserviceutil.la
libSyslog_ServiceUtils_la_SOURCES= Syslog_ServiceUtils.c
libSyslog_ServiceUtils_la_LDFLAGS = -Wc,-nostartfiles
libsyslogserviceutil_la_SOURCES = util/syslogserviceutil.c
libsyslogserviceutil_la_LDFLAGS = -Wc,-nostartfiles

noinst_HEADERS = Syslog_ServiceUtils.h util/syslogserviceutil.h

EXTRA_DIST+=README
#
# Mof file
#
SCHEMAS=mof/Syslog_Service.mof
REGISTRATIONS=mof/Syslog_Service.registration

pkgdata_DATA=$(SCHEMAS) $(REGISTRATIONS)
pkgdata_SCRIPTS=provider-register.sh

EXTRA_DIST+=mof $(pkgdata_SCRIPTS)
# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	sh provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)
preuninstall:
	sh provider-register.sh -d -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)


# Install util/syslog-service.sh
bin_SCRIPTS = util/syslog-service.sh
EXTRA_DIST+=$(bin_SCRIPTS)

#
# Automake instructions for ./test subdir
#
EXTRA_DIST+=test
if TESTSUITE
testsuitedir=@TESTSUITEDIR@
testsuitesystemdir=@TESTSUITEDIR@/system/linux
testsuitecimdir=@TESTSUITEDIR@/cim

testsuite_SCRIPTS=test/test-cmpi-syslog-service.sh \
		test/runtest_wbemcli.sh \
		test/runtest_pegasus.sh
testsuitesystem_DATA=test/system/linux/Syslog_Service.system
testsuitecim_DATA=test/cim/Syslog_Service.cim

runtest: install
	cd $(DESTDIR)$(TESTSUITEDIR) && ./test-cmpi-syslog-service.sh

doc_DATA+=test/readme.test-syslog-service
endif

