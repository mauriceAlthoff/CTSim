bin_PROGRAMS = @lamprograms@  ctsimtext 
EXTRA_PROGRAMS = ctsimtext-lam
INCLUDES=@my_includes@

if HAVE_SGP
SOURCE_DEPEND=../include/ct.h ../libctsim/libctsim.a ../libctsupport/libctsupport.a ../libctgraphics/libctgraphics.a
else
SOURCE_DEPEND=../include/ct.h ../libctsim/libctsim.a ../libctsupport/libctsupport.a
endif

install-exec-hook:
	ln -sf ctsimtext $(bindir)/if1
	ln -sf ctsimtext $(bindir)/if2
	ln -sf ctsimtext $(bindir)/ifexport
	ln -sf ctsimtext $(bindir)/ifinfo
	ln -sf ctsimtext $(bindir)/phm2if
	ln -sf ctsimtext $(bindir)/phm2pj
	ln -sf ctsimtext $(bindir)/phm2helix
	ln -sf ctsimtext $(bindir)/pj2if
	ln -sf ctsimtext $(bindir)/pjinfo
	ln -sf ctsimtext $(bindir)/pjrec
	ln -sf ctsimtext $(bindir)/pjHinterp
	ln -sf ctsimtext $(bindir)/linogram

ctsimtext_SOURCES = ctsimtext.cpp if1.cpp if2.cpp ifinfo.cpp ifexport.cpp phm2if.cpp phm2pj.cpp pj2if.cpp pjinfo.cpp pjrec.cpp nographics.cpp phm2helix.cpp pjHinterp.cpp linogram.cpp
ctsimtext_LDADD=@ctlibs@
ctsimtext_DEPENDENCIES=$(SOURCE_DEPEND)

CLEANFILES=\*.pgm \*.if \*~ \*.pj sample-ctsim.sh

ctsimtext_lam_SOURCES = ctsimtext.cpp if1.cpp if2.cpp ifinfo.cpp ifexport.cpp phm2if.cpp phm2pj.cpp pj2if.cpp pjinfo.cpp pjrec.cpp nographics.cpp mpiworld.cpp phm2helix.cpp pjHinterp.cpp linogram.cpp
ctsimtext_lam_LDADD=@ctlamlibs@

if USE_LAM
CC_LAM = $(lamdir)/bin/hcp
LAM_EXTRA_SRC = mpiworld.cpp

ctsimtext_lam_DEPENDENCIES=$(SOURCE_DEPEND)

ctsimtext-lam$(EXEEXT): ctsimtext.cpp if1.cpp if2.cpp ifinfo.cpp ifexport.cpp phm2if.cpp phm2pj.cpp pj2if.cpp pjinfo.cpp pjrec.cpp mpiworld.cpp linogram.cpp ../include/ct.h ../libctsim/libctsim.a ../libctsupport/libctsupport.a
	$(CC_LAM) @DEFS@ @lamdefs@ $(CFLAGS) $(INCLUDES) -DHAVE_MPI -DNO_MAIN ctsimtext.cpp if1.cpp if2.cpp ifinfo.cpp ifexport.cpp phm2if.cpp phm2pj.cpp pj2if.cpp pjinfo.cpp pjrec.cpp -o ctsimtext-lam $(LDFLAGS) $(LAM_EXTRA_SRC) @ctlibs@

endif



