bin_SCRIPTS = sort.dvi cdb.dvi

DESTDIR=${prefix}

sort.tex: ../bin/ldoc ../include/sort.h ../lib/sort.c
	../bin/ldoc -of sort.tex -t "A sort algorithm" -a "by David F. May" \
                ../include/sort.h

cdb.tex: ../bin/ldoc ../include/sort.h ../lib/sort.c
	../bin/ldoc -of cdb.tex -t "DCDB database engine" -a "by David F. May" \
		../include/cdb.h

sort.dvi: sort.tex
	-latex sort
	-latex sort
	-makeindex sort
	-latex sort
	-rm -f sort.aux sort.idx sort.ilg sort.ind sort.lj sort.log sort.toc

cdb.dvi: cdb.tex
	-latex cdb
	-latex cdb
	-makeindex cdb
	-latex cdb
	-rm -f cdb.aux cdb.idx cdb.ilg cdb.ind cdb.lj cdb.log cdb.toc

clean:
	rm -f cdb.tex cdb.dvi sort.tex sort.dvi


install: sort.dvi cdb.dvi
	$(mkinstalldirs) $(DESTDIR)/doc
	$(INSTALL) -m 0644 cdb.dvi $(DESTDIR)/doc
	$(INSTALL) -m 0644 cdb.tex $(DESTDIR)/doc
	$(INSTALL) -m 0644 sort.dvi $(DESTDIR)/doc
	$(INSTALL) -m 0644 sort.tex $(DESTDIR)/doc

pkgincludedir=$(includedir)
