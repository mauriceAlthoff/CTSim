docs := ctsim.tex texhelp.sty psbox.tex logo.eps logo.gif ctsim-sgp.tex ctsim-concepts.tex ctsim-gui.tex ctsim-install.tex ctsim-textui.tex ctsim-web.tex alphacalc.eps alphacalc.gif divergent.eps divergent.gif equiangular.eps equiangular.gif equilinear.eps equilinear.gif scangeometry.eps scangeometry.gif tex2rtf.ini ctsim.hpj bullet.bmp verbatim.sty mytitle.sty makeidx.sty mysober.sty fancyhea.sty ctsim-latex.bib ctsim-tex2rtf.bib

pkg=ctsim

all: htmlhelp html pdf 

CLEANFILES=$(pkg).ps $(pkg).hhc $(pkg).hhp $(pkg).hhk $(pkg).bib $(pkg).zip $(pkg).ilg $(pkg).ind $(pkg).dvi $(pkg).bb $(pkg).blg $(pkg).idx $(pkg).toc $(pkg).bbl $(pkg).con $(pkg).ref $(pkg).blg

clean:
	@rm -f $(CLEANFILES) $(pkg)*.html *.aux *.log *~ '#*~' '#*#' .*~
	@rm -rf html

.PHONY: pdf
pdf: ../docs/$(pkg).pdf

../docs/$(pkg).pdf: $(docs)
	$(MAKE) dvi
	dvipdfm $(pkg)
	mv $(pkg).pdf ../docs

#.PHONY: ps
#ps: $(pkg).ps
#$(pkg).ps: $(pkg).pdf
#	pdftops $(pkg).pdf $(pkg).ps

.PHONY: dvi
dvi: $(pkg).dvi
$(pkg).dvi: $(docs)
	cp $(pkg)-latex.bib $(pkg).bib
	latex $(pkg)
	bibtex $(pkg) > /dev/null
	latex $(pkg)
	makeindex $(pkg) > /dev/null
	latex $(pkg)

.PHONY: htmlhelp
htmlhelp: ../docs/$(pkg).htb

../docs/$(pkg).htb: $(docs)
	rm -f $(pkg)*.html $(pkg).hh[pck] $(pkg).zip
	cp $(pkg)-tex2rtf.bib $(pkg).bib
	tex2rtf $(pkg).tex $(pkg).html -html  > /dev/null
	./maphhc.pl
	zip -q $(pkg).zip *.html *.gif $(pkg).hh[pck]
	mv $(pkg).zip $(pkg).htb
	mv $(pkg).htb ../docs

.PHONY: html
html: ../docs/html.tar.gz

../docs/html.tar.gz: $(docs)
	rm -f $(pkg)*.html
	cp $(pkg)-tex2rtf.bib $(pkg).bib
	tex2rtf $(pkg).tex $(pkg).html -html  > /dev/null
	rm -rf html
	mkdir html
	cp *.gif html
	mv $(pkg)*.html html
	GZIP='-9' tar czf html.tar.gz html
	mv html.tar.gz ../docs
	rm -r html

#.PHONY: winhelp
#winhelp: $(docs)
#	rm -f $(pkg).rtf $(pkg).cnt
#	cp $(pkg)-tex2rtf.bib $(pkg).bib
#	tex2rtf $(pkg).tex $(pkg).rtf -winhelp
#	./maphhc.pl
#	hcw /c /m /e $(pkg).hpj
#	mv $(PKG).HLP $(pkg).hlp
#	cp $(pkg).hlp $(pkg).cnt ../docs/$(pkg)/

