
Name:	org.tizen.download-manager
Summary:	Application for support of the content download
Version:	0.1.6
Release:	1
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
chown 5000:5000 /opt/usr/apps/org.tizen.download-manager/data/db/.download-history.db*
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
* Fri Jun 28 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Change the default saved path as default storage setting

* Wed Jun 26 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Merge latest codes from private branch
- Apply v1.7 UI guide (include back key, black theme etc)

* Fri Mar 10 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Modify e17 and notification smac rule
- Apply to sign application packages

* Fri Mar 10 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Add smack rule file.

* Tue Mar 05 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Add function to handle credential URL
- Add patch about screen reader function.
- Enable visibility hidden opetion

* Tue Jan 29 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Modify manifest for destkop file
- Change the point to create db
- Add dependency about db module
- Add downloading information to db

* Wed Jan 16 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Resolve a bug about cancel operation

* Tue Jan 08 2013 Jungki Kwak <jungki.kwak@samsung.com>
- Resolve prevent defects
- Fixed a bug that the icon of txt file can't be recognized and the txt file can't be opend
- Modify to check returned value of sqlite API
- Show error string in case of download CAPI failure

* Thu Dec 27 2012 Jungki kwak <jungki.kwak@samsung.com>
- Resolve prevent defects

* Thu Dec 20 2012 Jungki kwak <jungki.kwak@samsung.com>
- Add the function about silet mode
- Apply latest GUI guide
- Change to show information in case of download finish
- Remove duplicated dlog message

* Mon Dec 17 2012 Jungki kwak <jungki.kwak@samsung.com>
- The API for window activation is changed
- Add license information at spec file
- Rename es_US po file
- The function for handing saved content are changed because the temporary file is not used
- Resolve prevent defects
- Add to register thumbnail image for notification message

* Thu Dec 06 2012 Jungki kwak <jungki.kwak@samsung.com>
- Add popup message in case of no player
- Update string files

* Tue Dec 04 2012 Jungki kwak <jungki.kwak@samsung.com>
- Change the button and action in retry popup
- Resolve a bug about restarting oma download
- Apply the migration EFL API
- Add to retry download in case the downloaded file is not existed
- Prevent to show one more than OMA popups
- Add an action about puase event of the application
- Add to extract sender name form url
- Change the cancel callback function and flow
- Add error exception of failure the download CAPI
- Add error exception of download CAPI failure in CB
- Show no name title when title is empty in CAPI noti
- Remove duplicated update event in failure of download start

* Tue Nov 13 2012 Jungki kwak <jungki.kwak@samsung.com>
- Resolve a bug about mischaracter at xml file
- Revert to apply winsets of net style

* Thu Nov 08 2012 Jungki kwak <jungki.kwak@samsung.com>
- Change the alignment for select notify layout
- Apply changed enum value of connection CAPI

* Mon Nov 05 2012 Jungki kwak <jungki.kwak@samsung.com>
- Remove the select notify popup when the deleteion is selected from retry popup
- Apply changed UX concept about completed download item
- Apply new CAPIs of capi-web-url-download package
- Remove cancel button from delete list view

* Tue Oct 23 2012 Jungki kwak <jungki.kwak@samsung.com>
- According to UX guide 1.0
- Apply the status notify and the languge changed event
- Modify to update date and time of download item

* Fri Oct 12 2012 Jungki kwak <jungki.kwak@samsung.com>
- Install LICENSE file

* Fri Sep 21 2012 Jungki kwak <jungki.kwak@samsung.com>
- Apply a manifest file

* Tue Sep 14 2012 Jungki kwak <jungki.kwak@samsung.com>
- Add OMA download feature
- Fix the crash - try to drop uninitialized string

* Tue Sep 04 2012 Jungki kwak <jungki.kwak@samsung.com>
- Change the name of application operation

* Thu Aug 30 2012 Kwangmin Bang <justine.bang@samsung.com>
- fix display language build error

* Wed Aug 29 2012 Kwangmin Bang <justine.bang@samsung.com>
- add display language set

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

