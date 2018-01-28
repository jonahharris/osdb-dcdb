SUBDIRS = lib bin
EXTRA_DIST = $(top_srcdir)/sort $(top_srcdir)/extras $(top_srcdir)/contrib \
	     $(top_srcdir)/doc

ALL: lib/libcdb-$(PACKAGE_VERSION)$(shrext) lib/libcdb-$(PACKAGE_VERSION).a
	-cd lib
	-$(MAKE) libcdb-$(PACKAGE_VERSION).a
	-$(MAKE) libcdb-$(PACKAGE_VERSION)$(shrext)

realclean:
	-cd $(top_srcdir)/bin && $(MAKE) clean
	-cd $(top_srcdir)/doc && $(MAKE) clean
	-cd $(top_srcdir)/lib && $(MAKE) clean
	-cd $(top_srcdir)/tests && $(MAKE) cleandb && $(MAKE) clean
