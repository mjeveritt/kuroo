# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=4

inherit subversion kde4-base

DESCRIPTION="Grophical Portage frontend based on KDE4/Qt4"
HOMEPAGE="http://kuroo.sourceforge.net/"
SRC_URI=""
ESVN_REPO_URI="http://kuroo.svn.sourceforge.net/svnroot/kuroo/kuroo4/trunk"
ESVN_PROJECT="kuroo4"

LICENSE="GPL-2"
SLOT="4"
KEYWORDS=""
IUSE="debug"

DEPEND="app-portage/gentoolkit
	dev-db/sqlite
	$(add_kdebase_dep kdesu)
	$(add_kdebase_dep kompare)"
RDEPEND="${DEPEND}"
