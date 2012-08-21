
Name:	org.tizen.download-manager
Summary:	Application for support of the content download
Version:	0.0.1
Release:	4
Group:		TO_BE_FILLED_IN
License:	TO_BE_FILLED_IN
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
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(xdgmime)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: cmake
BuildRequires: gettext-devel
BuildRequires: expat-devel
BuildRequires: edje-tools

%description
Application for support of the content download

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX="/opt/apps/org.tizen.download-manager"

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
#### Download History ####
if [ ! -f /opt/apps/org.tizen.download-manager/data/db/.download-history.db ];
then
		sqlite3 /opt/apps/org.tizen.download-manager/data/db/.download-history.db 'PRAGMA journal_mode=PERSIST;
		create table history(id integer primary key autoincrement, historyid integer, downloadtype integer, contenttype integer, state integer, err integer, name, path, url, cookie, date datetime);'
fi

chown -R 5000:5000 /opt/apps/org.tizen.download-manager/data
chmod 660 /opt/apps/org.tizen.download-manager/data/db/.download-history.db
chmod 660 /opt/apps/org.tizen.download-manager/data/db/.download-history.db-journal

%files
%defattr(-,root,root,-)
/opt/apps/org.tizen.download-manager/bin/*
/opt/apps/org.tizen.download-manager/data
/opt/apps/org.tizen.download-manager/res/*
/opt/share/packages/org.tizen.download-manager.xml

%changelog
* Mon Aug 17 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Add https protocol to service uri field

* Mon Aug 16 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Change boilerplates and licence to flora

* Mon Aug 10 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Do not display installing message

* Fri Aug 03 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Initial release

