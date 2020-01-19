# SBLIM Include Directory
sblimincdir = $(includedir)/sblim

# Start from an empty extra distribution file list
EXTRA_DIST=
doc_DATA=

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script).
providerdir = @PROVIDERDIR@

provider_LTLIBRARIES = libSyslog_MessageLog.la \
		libSyslog_LogRecord.la \
		libSyslog_RecordInLog.la

libSyslog_MessageLog_la_SOURCES = Syslog_MessageLog.c
libSyslog_MessageLog_la_LIBADD = -lSyslog_LogUtils -lsysloglogutil -lsyslogtimeparse
libSyslog_MessageLog_la_LDFLAGS = -avoid-version

libSyslog_LogRecord_la_SOURCES = Syslog_LogRecord.c
libSyslog_LogRecord_la_LIBADD = -lSyslog_LogUtils -lsysloglogutil -lsyslogtimeparse
libSyslog_LogRecord_la_LDFLAGS = -avoid-version

libSyslog_RecordInLog_la_SOURCES = Syslog_RecordInLog.c
libSyslog_RecordInLog_la_LIBADD = -lSyslog_LogUtils -lsysloglogutil -lsyslogtimeparse
libSyslog_RecordInLog_la_LDFLAGS = -avoid-version

lib_LTLIBRARIES = libSyslog_LogUtils.la libsysloglogutil.la libsyslogtimeparse.la
libSyslog_LogUtils_la_SOURCES= Syslog_LogUtils.c 
libSyslog_LogUtils_la_LDFLAGS = -Wc,-nostartfiles
libsysloglogutil_la_SOURCES = util/sysloglogutil.c
libsysloglogutil_la_LDFLAGS = -Wc,-nostartfiles
libsyslogtimeparse_la_SOURCES = util/syslogtimeparse.c
libsyslogtimeparse_la_LDFLAGS = -Wc,-nostartfiles

noinst_HEADERS = Syslog_LogUtils.h util/sysloglogutil.h
EXTRA_DIST+=README
#
# Mof file
#
SCHEMAS=mof/Syslog_Log.mof
REGISTRATIONS=mof/Syslog_Log.registration

pkgdata_DATA=$(SCHEMAS) $(REGISTRATIONS)
pkgdata_SCRIPTS=provider-register.sh

EXTRA_DIST+=mof $(pkgdata_SCRIPTS)
# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	sh provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)
preuninstall:
	sh provider-register.sh -d -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)

#
# Automake instructions for ./test subdir
#
EXTRA_DIST+=test
if TESTSUITE
testsuitedir=@TESTSUITEDIR@
testsuitesystemdir=@TESTSUITEDIR@/system/linux
testsuitecimdir=@TESTSUITEDIR@/cim

testsuite_SCRIPTS=test/test-cmpi-syslog-log.sh \
		test/runtest_wbemcli.sh \
		test/runtest_pegasus.sh

testsuitesystem_DATA=test/system/linux/Syslog_LogRecord.system \
		test/system/linux/Syslog_MessageLog.system \
		test/system/linux/Syslog_RecordInLog.system

testsuitesystem_SCRIPTS=test/system/linux/logrecord.sh \
		test/system/linux/messagelog.sh \
		test/system/linux/msglogtest.sh \
		test/system/linux/Syslog_MessageLog.pl

testsuitecim_DATA=test/cim/Syslog_LogRecord.cim \
		test/cim/Syslog_MessageLog.cim \
		test/cim/Syslog_RecordInLog.cim

runtest: install
	cd $(DESTDIR)$(TESTSUITEDIR) && ./test-cmpi-syslog-log.sh

doc_DATA+=test/readme.test-syslog-log
endif

