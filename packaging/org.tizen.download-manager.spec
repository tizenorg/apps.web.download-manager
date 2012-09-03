
Name:	org.tizen.download-manager
Summary:	Application for support of the content download
Version:	0.0.1
Release:	11
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
/opt/apps/org.tizen.download-manager/res/edje/*
/opt/share/packages/org.tizen.download-manager.xml

%changelog
* Mon Sep 03 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Add deisplay language set
- Resolve a bug about the name of edc

* Mon Aug 27 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Change notify winset to selectioninfo winset
- Apply changes of capi-web-url-download
- Resolve a prevent defect

* Wed Aug 22 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Enable to use notification of download CAPI
- Show the application in case of default service operation

* Mon Aug 17 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Add https protocol to service uri field

* Mon Aug 16 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Apply changes of capi-web-url-download
- Change boilerplates and licence to flora

* Mon Aug 10 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Do not display installing message

* Fri Aug 03 2012 Jungki Kwak <jungki.kwak@samsung.com>
- Initial release

