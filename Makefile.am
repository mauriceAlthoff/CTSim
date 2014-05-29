if INCLUDED_GETOPT_LONG
EXTRA_DIRS1=getopt
else
EXTRA_DIRS1=
endif

if HAVE_SGP
EXTRA_DIRS2=libctgraphics
else
EXTRA_DIRS2=
endif

if HAVE_WXWINDOWS
EXTRA_DIRS3=src
else
EXTRA_DIRS3=
endif

SUBDIRS=man libctsupport libctsim  html cgi-bin include $(EXTRA_DIRS1) $(EXTRA_DIRS2) $(EXTRA_DIRS3) tools helical

EXTRA_DIST=acsite.m4 make.bat msvc/ctsim.dsw msvc/ctsimtext/ctsimtext.dsp msvc/libctsim/libctsim.dsp msvc/ctsim/ctsim.dsp

CLEANFILES=docs/html/\* config.log config.status configure.in config.h stamp-h1
clean-local:
	rm -rf docs/html

#install-data-hook:
#	test -d $(datadir) || mkdir -p $(datadir) && chmod 0755 $(datadir)
#	install -g root -o root -m 0644 docs/ctsim.htb $(datadir)





