
Name:       ui-gadget-1
Summary:    UI Gadget Library
Version:    0.1.12
Release:    1
Group:      System/Libraries
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  cmake
BuildRequires:  edje-bin

%description
UI gadget library (development headers)

%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
%description devel
Development files for %{name}

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=/usr

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post 
/sbin/ldconfig
ln -sf /usr/bin/ug-client /usr/bin/ug-launcher

%postun -p /sbin/ldconfig

%files
%manifest ui-gadget-1.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_libdir}/lib%{name}-efl-engine.so
/usr/share/edje/ug_effect.edj
%{_bindir}/ug-client
/usr/share/edje/ug-client/*.edj

%files devel
%defattr(-,root,root,-)
%{_includedir}/ug-1/*.h
%{_libdir}/libui-gadget-1.so
%{_libdir}/pkgconfig/%{name}.pc

