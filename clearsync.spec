# ClearSync RPM spec
Name: clearsync
Version: 2.0
Release: 5%{dist}
Vendor: ClearFoundation
License: GPL
Group: System Environment/Daemons
Packager: ClearFoundation
Source: %{name}-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}
Summary: ClearSync system sychronization daemon
BuildRequires: autoconf >= 2.63
BuildRequires: automake
BuildRequires: libtool
BuildRequires: expat-devel openssl-devel
%if "0%{dist}" != "0.v6"
BuildRequires: systemd
%{?systemd_requires}
%endif
%if "0%{dist}" == "0.v6"
Requires: initscripts /sbin/service
Requires(post): /sbin/chkconfig
Requires(preun): /sbin/chkconfig
%endif
Requires(pre): /sbin/ldconfig, /usr/sbin/useradd, /usr/bin/getent
Requires(postun): /usr/sbin/userdel

%description
ClearSync system synchronization daemon
Report bugs to: http://www.clearfoundation.com/docs/developer/bug_tracker/

%package devel
Summary: ClearSync plugin development files
Group: Development/Libraries
Requires: clearsync = 2.0
BuildArch: noarch

%description devel
ClearSync plugin development files
Report bugs to: http://www.clearfoundation.com/docs/developer/bug_tracker/

# Build
%prep
%setup -q
./autogen.sh
%{configure}

%build
make %{?_smp_mflags}

# Install
%install
make install DESTDIR=%{buildroot}
mkdir -vp %{buildroot}/%{_sysconfdir}/clearsync.d
mkdir -vp %{buildroot}/%{_localstatedir}/run/clearsync
mkdir -vp %{buildroot}/%{_localstatedir}/state/clearsync
mkdir -vp %{buildroot}/%{_localstatedir}/lib/clearsync

install -D -m 644 sysconf/clearsync.conf %{buildroot}/%{_sysconfdir}/clearsync.conf

%if "0%{dist}" == "0.v6"
install -D -m 644 sysconf/clearsyncd.init %{buildroot}/%{_sysconfdir}/init.d/clearsyncd
%else
install -D -m 644 sysconf/clearsync.service %{buildroot}/%{_unitdir}/clearsync.service
install -D -m 644 sysconf/clearsync.tmp %{buildroot}/%{_tmpfilesdir}/clearsync.conf
%endif

# Clean-up
%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

# Pre install
%pre
/usr/bin/getent passwd clearsync 2>&1 >/dev/null ||\
    /usr/sbin/useradd -M -c "ClearSync" -r -d %{_sbindir}/clearsyncd -s /bin/false clearsync 2> /dev/null || :

%preun
%if "0%{dist}" == "0.v6"
if [ "$1" = 0 ]; then
    /sbin/chkconfig --del clearsyncd
fi
%else
if [ "$1" = 0 ]; then
    /sbin/chkconfig --del clearsync
    /usr/bin/systemctl stop clearsync.service -q
    /usr/bin/systemctl disable clearsync.service -q
fi
%endif

# Post install
%post
/sbin/ldconfig
%if "0%{dist}" == "0.v6"
/sbin/chkconfig --add clearsyncd >/dev/null 2>&1 || :
/sbin/service clearsyncd condrestart >/dev/null 2>&1 || :
%else
/sbin/chkconfig --add clearsync >/dev/null 2>&1 || :
/usr/bin/systemctl enable clearsync.service -q
/usr/bin/systemctl reload-or-restart clearsync.service -q
%endif

# Post uninstall
%postun
/sbin/ldconfig
%if "0%{dist}" == "0.v6"
if [ -f /var/lock/subsys/clearsyncd ]; then
    killall -TERM clearsyncd 2>&1 >/dev/null || :
    sleep 2
fi
%else
%systemd_postun_with_restart %{name}.service
%endif

# Files
%files
%defattr(-,root,root)
%if "0%{dist}" == "0.v6"
%attr(755,root,root) %{_sysconfdir}/init.d/%{name}
%else
%attr(644,root,root) %{_unitdir}/%{name}.service
%attr(644,root,root) %{_tmpfilesdir}/%{name}.conf
%endif
%{_sbindir}/clearsyncd
%{_libdir}/libclearsync.so*
%{_sysconfdir}/clearsync.d
%{_sysconfdir}/clearsync.conf
%attr(755,clearsync,clearsync) %{_localstatedir}/run/clearsync
%attr(750,clearsync,clearsync) %{_localstatedir}/state/clearsync
%attr(750,clearsync,clearsync) %{_localstatedir}/lib/clearsync

# Developer files
%files devel
%defattr(-,root,root)
%{_includedir}/clearsync
%{_libdir}/libclearsync.a
%{_libdir}/libclearsync.la

# vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
