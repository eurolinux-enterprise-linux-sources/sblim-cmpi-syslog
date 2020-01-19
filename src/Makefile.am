## Process this file with automake to produce Makefile.in

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_DATA =README
# We must explicity add the RPM spec file to the distribution package
EXTRA_DIST = $(PACKAGE).spec $(PACKAGE).rh.spec

postinstall:
	make -C syslog-conf postinstall
	make -C syslog-log postinstall
	make -C syslog-service postinstall

preuninstall:
	make -C syslog-conf preuninstall
	make -C syslog-log preuninstall
	make -C syslog-service preuninstall

SUBDIRS = syslog-conf syslog-log syslog-service
