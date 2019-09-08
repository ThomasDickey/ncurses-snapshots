Summary: Ada95 binding for ncurses
%define AppProgram AdaCurses
%define AppVersion MAJOR.MINOR
%define AppRelease YYYYMMDD
# $Id: AdaCurses.spec,v 1.19 2019/09/08 01:00:18 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: %{AppRelease}
License: MIT
Group: Applications/Development
URL: ftp://ftp.invisible-island.net/%{AppProgram}
Source0: %{AppProgram}-%{AppRelease}.tgz
Packager: Thomas Dickey <dickey@invisible-island.net>

%description
This is the Ada95 binding from the ncurses MAJOR.MINOR distribution, for
patch-date YYYYMMDD.

In addition to a library, this package installs sample programs in
"bin/%{AppProgram}" to avoid conflict with other packages.
%prep

%define debug_package %{nil}

# http://fedoraproject.org/wiki/EPEL:Packaging_Autoprovides_and_Requires_Filtering
%filter_from_requires /lib%{AppProgram}.so.1/d
%filter_setup

%setup -q -n %{AppProgram}-%{AppRelease}

%build

%define ada_libdir %{_prefix}/lib/ada/adalib
%define ada_include %{_prefix}/share/ada/adainclude

INSTALL_PROGRAM='${INSTALL}' \
	./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--with-ada-include=%{ada_include} \
		--with-ada-objects=%{ada_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir} \
		--disable-rpath-link \
		--with-shared \
		--with-ada-sharedlib

make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install               DESTDIR=$RPM_BUILD_ROOT

( cd samples &&
  make install.examples \
  	DESTDIR=$RPM_BUILD_ROOT \
	BINDIR=$RPM_BUILD_ROOT%{_bindir}/%{AppProgram}
)

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/adacurses*-config
%{_bindir}/%{AppProgram}/*
%{ada_libdir}/lib%{AppProgram}.*
%{_mandir}/man1/adacurses*-config.1*
%{_datadir}/%{AppProgram}/*
%{ada_include}/terminal_interface*

%changelog
# each patch should add its ChangeLog entries here

* Sat Sep 07 2019 Thomas Dickey
- use AppProgram to replace "AdaCurses" globally
- amend install-paths to work with Fedora30

* Thu Mar 31 2011 Thomas Dickey
- use --with-shared option for consistency with --with-ada-sharelib
- ensure that MY_DATADIR is set when installing examples
- add ada_libdir symbol to handle special case where libdir is /usr/lib64
- use --disable-rpath-link to link sample programs without rpath

* Fri Mar 25 2011 Thomas Dickey
- initial version
