%define _ux_define tizen2.3
Name:	org.tizen.download-manager
Summary:	Application for support of the content download
Version:	0.3.9
Release:	1
License:	Flora-1.1
Group:		misc
URL:		N/A
Source0:	%{name}-%{version}.tar.gz
BuildRequires: pkgconfig(capi-web-url-download)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-content-mime-type)
BuildRequires: pkgconfig(capi-base-utils-i18n)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(gles20)
BuildRequires: pkgconfig(xdgmime)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(libxml-2.0)
BuildRequires: pkgconfig(storage)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: cmake
BuildRequires: gettext-devel
BuildRequires: expat-devel
BuildRequires: hash-signer
Requires(post): coreutils
Requires(post): sqlite
Requires(post): edje-tools

%{echo:#winset style:%{?_ux_define}#}

%description
Application for support of the content download

%prep
%setup -q

%define _default_path /usr/apps/%{name}
%define _bin_path %{_default_path}/bin
%define _res_path %{_default_path}/shared/res
%define _imagedir %{_res_path}/images
%if "%{?_ux_define}" == "tizen2.3"
%define _edjedir %{_res_path}/edje
%define _tabledir %{_res_path}/tables
%endif
%define _localedir %{_res_path}/locale
%define _pkgxmldir /usr/share/packages
%define _icondir /usr/share/icons/default/small
%define _licensedir /usr/share/license
%define support_oma_drm OFF

%define cmake \
	CFLAGS="${CFLAGS:-%optflags} -fPIC -D_REENTRANT -fvisibility=hidden"; export CFLAGS \
	CXXFLAGS="${CXXFLAGS:-%optflags} -fvisibility-inlines-hidden"; export CXXLAGS \
	FFLAGS="${FFLAGS:-%optflags} -fPIC -fvisibility=hidden"; export FFLAGS \
	LDFLAGS+=" -Wl,--as-needed -Wl,--hash-style=both"; export LDFLAGS \
	%__cmake \\\
		-DCMAKE_INSTALL_PREFIX:PATH=%{_default_path} \\\
		%if "%{?_lib}" == "lib64" \
		%{?_cmake_lib_suffix64} \\\
		%endif \
		%{?_cmake_skip_rpath} \\\
		%if "%{?support_oma_drm}" == "ON" \
		-DSUPPORT_WAITING_RO:BOOL=ON \\\
		%else \
		-DSUPPORT_WAITING_RO:BOOL=OFF \\\
		%endif \
		%if "%{?_ux_define}" == "tizen2.3" \
		-DTIZEN_2_3_UX:BOOL=ON \\\
		%endif \
		-DPKG_NAME=%{name} \\\
		-DPKG_VERSION=%{version} \\\
		-DPKG_RELEASE=%{release}


%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%cmake .
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
PKG_ID=%{name}
%define tizen_sign 1
%define tizen_sign_base /usr/apps/${PKG_ID}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
mkdir -p %{buildroot}/usr/share/license

%post
#mkdir -p /opt/usr/apps/org.tizen.download-manager/data/db
#chown -R 5000:5000 /opt/usr/apps/org.tizen.download-manager/data
#chmod -R 755 /opt/usr/apps/org.tizen.download-manager/data
#chsmack -a 'org.tizen.download-manager' /opt/usr/apps/org.tizen.download-manager/data/db

#chsmack -a 'System::Shared' /opt/usr/apps/org.tizen.download-manager/data/db
#chsmack -t /opt/usr/apps/org.tizen.download-manager/data/db

%files
%defattr(-,root,root,-)
%manifest %{name}.manifest
%{_bin_path}/*
%{_imagedir}/*
%if "%{?_ux_define}" == "tizen2.3"
%{_edjedir}/*
%{_tabledir}/*
%endif
%{_localedir}/*
%{_default_path}/author-signature.xml
%{_default_path}/signature1.xml
%{_pkgxmldir}/%{name}.xml
%{_icondir}/%{name}.png
%{_licensedir}/%{name}

