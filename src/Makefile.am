INCLUDES=@my_includes@

bin_PROGRAMS=ctsim

ctsim_SOURCES=ctsim.cpp docs.cpp views.cpp dialogs.cpp ctsim.h docs.h \
views.h dialogs.h dlgprojections.cpp dlgprojections.h dlgreconstruct.cpp \
dlgreconstruct.h  graph3dview.cpp graph3dview.h ctsim-map.h ctsim.xpm \
splash.xpm tips.cpp tips.h threadrecon.cpp threadrecon.h backgroundmgr.cpp \
backgroundmgr.h backgroundsupr.cpp backgroundsupr.h threadproj.cpp \
threadproj.h threadraster.cpp threadraster.h dlgezplot.cpp dlgezplot.h

ctsim_DEPENDENCIES=../libctgraphics/libctgraphics.a ../libctsupport/libctsupport.a ../libctsim/libctsim.a ../include/ct.h
ctsim_LDADD=-L../libctgraphics -L../libctsupport -L../libctsim @ctlibs@
