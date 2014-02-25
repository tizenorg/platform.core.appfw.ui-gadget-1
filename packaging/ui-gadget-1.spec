%bcond_with x
%bcond_with wayland

Name:       ui-gadget-1
Summary:    UI Gadget Library
Version:    0.1.30
Release:    1
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	ui-gadget-1.manifest
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(dlog)
%if %{with x}
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(x11)
%endif
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  cmake
BuildRequires:  edje-bin
BuildRequires: pkgconfig(libtzplatform-config)

%description
UI gadget library (development headers)

%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
%if %{with x}
Requires:  pkgconfig(x11)
%endif

%description devel
Development files for %{name}

%prep
%setup -q
cp %{SOURCE1001} .

%build
%cmake . \
-DTZ_SYS_ETC=%TZ_SYS_ETC \
%if %{with wayland} && !%{with x}
-Dwith_wayland=TRUE
%else
-Dwith_x=TRUE
%endif

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
install LICENSE %{buildroot}/usr/share/license/%{name}

%post
/sbin/ldconfig
ln -sf %{TZ_SYS_BIN}/ug-client %{TZ_SYS_BIN}/ug-launcher

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_libdir}/lib%{name}-efl-engine.so
/usr/share/edje/ug_effect.edj
%{_bindir}/ug-client
/usr/share/edje/ug-client/*.edj
%{TZ_SYS_ETC}/smack/accesses.d/ui-gadget-1.rule
/usr/share/license/%{name}

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/ug-1/*.h
%{_libdir}/libui-gadget-1.so
%{_libdir}/pkgconfig/%{name}.pc

