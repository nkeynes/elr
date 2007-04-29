## Process this file with automake to produce Makefile.in

SUBDIRS = src po

EXTRA_DIST = \
	autogen.sh

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/src/skel; then \
	  $(mkinstalldirs) $(DESTDIR)$(datadir)/$(PACKAGE)/skel; \
	  for file in $(srcdir)/src/skel/*; do \
	    if test -f $$file; then \
	      $(INSTALL_DATA) $$file $(DESTDIR)$(datadir)/$(PACKAGE)/skel; \
	    fi \
	  done \
	fi

dist-hook:
	if test -d skel; then \
	  mkdir $(distdir)/skel; \
	  for file in skel/*; do \
	    if test -f $$file; then \
	      cp -p $$file $(distdir)/skel; \
	    fi \
	  done \
	fi

