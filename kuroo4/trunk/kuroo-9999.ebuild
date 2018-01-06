# Copyright 1999-2015 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=6

inherit kde5 subversion

DESCRIPTION="Graphical Portage frontend based on KF5/Qt5"
HOMEPAGE="http://kuroo.sourceforge.net/"
ESVN_REPO_URI="http://kuroo.svn.sourceforge.net/svnroot/kuroo/kuroo4/trunk"
ESVN_PROJECT="kuroo4"

LICENSE="GPL-2"
KEYWORDS=""
SLOT="5"
IUSE="debug"

DEPEND="
	dev-db/sqlite:3
"
RDEPEND="${DEPEND}
	app-portage/gentoolkit
	$(add_qt_dep qtwidgets)
	$(add_frameworks_dep extra-cmake-modules)
	$(add_frameworks_dep kconfig)
	$(add_frameworks_dep kconfigwidgets)
	$(add_frameworks_dep kcoreaddons)
	$(add_frameworks_dep kdesu)
	$(add_frameworks_dep kdelibs4support)
	$(add_frameworks_dep ki18n)
	$(add_frameworks_dep kiconthemes)
	$(add_frameworks_dep kio)
	$(add_frameworks_dep kitemmodels)
	$(add_frameworks_dep kitemviews)
	$(add_frameworks_dep kwidgetsaddons)
	$(add_frameworks_dep kwindowsystem)
	$(add_frameworks_dep kxmlgui)
	$(add_frameworks_dep threadweaver)
	$(add_kdeapps_dep kompare)
"
