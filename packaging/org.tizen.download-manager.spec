
Name:	org.tizen.download-manager
Summary:	Application for support of the content download
Version:	0.1.9
Release:	0
Group:		misc
License:	Flora License, Version 1.1
URL:		N/A
Source0:	%{name}-%{version}.tar.gz
BuildRequires: pkgconfig(capi-web-url-download)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(xdgmime)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(utilX)
BuildRequires: pkgconfig(xproto)
BuildRequires: pkgconfig(x11)
BuildRequires: efl-assist-devel
BuildRequires: cmake
BuildRequires: gettext-devel
BuildRequires: expat-devel
BuildRequires: hash-signer
BuildRequires: libprivilege-control-conf
Requires(post): coreutils
Requires(post): sqlite
Requires(post): sys-assert

%description
Application for support of the content download

%prep
%setup -q

%build
CFLAGS+=" -fvisibility=hidden"; export CFLAGS;
CXXFLAGS+=" -fvisibility=hidden -fvisibility-inlines-hidden"; export CXXFLAGS;
cmake . -DCMAKE_INSTALL_PREFIX="/usr/apps/org.tizen.download-manager"

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
PKG_ID=%{name}
%define tizen_sign 1
%define tizen_sign_base /usr/apps/${PKG_ID}
%define tizen_sign_level public
%define tizen_author_sign 1
%define tizen_dist_sign 1
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/opt/usr/apps/org.tizen.download-manager/data/db
#### Download History ####
if [ ! -f %{buildroot}/opt/usr/apps/org.tizen.download-manager/data/db/.download-history.db ];
then
		sqlite3 %{buildroot}/opt/usr/apps/org.tizen.download-manager/data/db/.download-history.db 'PRAGMA journal_mode=PERSIST;
		create table history(id integer primary key autoincrement, downloadid integer, historyid integer, downloadtype integer,
		contenttype integer, state integer, err integer, name, path, url, cookie, headerfield, headervalue, installdir,
		installnotifyurl, date datetime);
		create index history_date_index on history (date);'
fi

%post
chown -R 5000:5000 /opt/usr/apps/org.tizen.download-manager/data
chmod -R 755 /opt/usr/apps/org.tizen.download-manager/data
chmod 660 /opt/usr/apps/org.tizen.download-manager/data/db/.download-history.db*

%files
%defattr(-,root,root,-)
%manifest org.tizen.download-manager.manifest
/usr/apps/org.tizen.download-manager/bin/*
/usr/apps/org.tizen.download-manager/res/*
/usr/apps/org.tizen.download-manager/res/locale/*/*/download-manager.mo
/usr/apps/org.tizen.download-manager/*.xml
/usr/share/packages/org.tizen.download-manager.xml
/usr/share/icons/default/small/org.tizen.download-manager.png
/usr/share/license/%{name}
/etc/smack/accesses.d/org.tizen.download-manager.rule
%attr(660,app,app) /opt/usr/apps/org.tizen.download-manager/data/db/.download-history.db*

%changelog
* Mon Aug 30 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Change to destroy only toolbar winset

* Thu Aug 22 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Update download icon for ongoing notification
- Remove function to receive network changed event
- Remove the function to get the default path from vconf
- Apply UI guide of version 2.0
- Restore the default value of silent mode at app paused state
- Resolve failed items of menu tree test

* Thu Aug 22 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Add view operation about all mime types
- Apply latest UI and GUI guide
- Update po files
- Default view modie is changed to hidden view

* Wed Jul 03 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Change directory access permission under opt

* Fri Jun 28 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Change the default saved path as default storage setting

* Wed Jun 26 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Merge latest codes from private branch
- Apply v1.7 UI guide (include back key, black theme etc)
