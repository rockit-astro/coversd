Name:      rockit-covers
Version:   %{_version}
Release:   1
Summary:   W1m mirror covers.
Url:       https://github.com/rockit-astro/coversd
License:   GPL-3.0
BuildArch: noarch

%description


%build
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_unitdir}
mkdir -p %{buildroot}/etc/bash_completion.d
mkdir -p %{buildroot}%{_sysconfdir}/coversd/
mkdir -p %{buildroot}%{_udevrulesdir}

%{__install} %{_sourcedir}/covers %{buildroot}%{_bindir}
%{__install} %{_sourcedir}/coversd %{buildroot}%{_bindir}
%{__install} %{_sourcedir}/coversd@.service %{buildroot}%{_unitdir}
%{__install} %{_sourcedir}/completion/covers %{buildroot}/etc/bash_completion.d
%{__install} %{_sourcedir}/onemetre.json %{buildroot}%{_sysconfdir}/coversd/

%package server
Summary:  Mirror covers control server.
Group:    Unspecified
Requires: python3-rockit-covers python3-pyserial
%description server

%files server
%defattr(0755,root,root,-)
%{_bindir}/coversd
%defattr(0644,root,root,-)
%{_unitdir}/coversd@.service

%package client
Summary:  Mirror covers control client.
Group:    Unspecified
Requires: python3-rockit-covers
%description client

%files client
%defattr(0755,root,root,-)
%{_bindir}/covers
/etc/bash_completion.d/covers

%package data-onemetre
Summary: Covers data for the W1m.
Group:   Unspecified
%description data-onemetre

%files data-onemetre
%defattr(0644,root,root,-)
%{_sysconfdir}/coversd/onemetre.json

%changelog
