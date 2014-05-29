# Version of CTSim
%define version 3.5.0

Summary: CTSim - Open-Source Computed Tomography Simulator
Name: ctsim
Version: %{version}
Release: 1
Packager: Kevin Rosenberg <kevin@rosenberg.net>
Source: ftp.ctsim.org:/pub/ctsim/ctsim-%{version}.tar.gz
Copyright: GPL
Group: Applications/Scientific

%description
CTSim is a Computed Tomography Simulator.

%prep
%setup 

%build
CFLAGS="$RPM_OPT_FLAGS" \
	./configure --prefix=/usr --enable-static --without-lam
make

%install
strip src/ctsim
strip tools/ctsimtext
install -s -m 755 -o 0 -g 0 src/ctsim /usr/bin/ctsim
install -s -m 755 -o 0 -g 0 tools/ctsimtext /usr/bin/ctsimtext
install -m 755 -o 0 -g 0 tools/sample-ctsim.sh /usr/bin/sample-ctsim.sh
mkdir -p /usr/share/ctsim
install -m 755 -o 0 -g 0 doc/ctsim.htb /usr/share/ctsim/ctsim.htb
ln -sf ctsimtext /usr/bin/if1
ln -sf ctsimtext /usr/bin/if2
ln -sf ctsimtext /usr/bin/ifexport
ln -sf ctsimtext /usr/bin/ifinfo
ln -sf ctsimtext /usr/bin/phm2if
ln -sf ctsimtext /usr/bin/phm2pj
ln -sf ctsimtext /usr/bin/pj2if
ln -sf ctsimtext /usr/bin/pjinfo
ln -sf ctsimtext /usr/bin/pjrec

%files
%doc NEWS README COPYING ChangeLog INSTALL

/usr/bin/ctsim
/usr/bin/ctsimtext
/usr/share/ctsim/ctsim.htb
/usr/share/ctsim/sample-ctsim.sh


