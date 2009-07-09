# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"

ARTS_REQUIRED="never"

LANGS="de es fr it nb pl ru"

inherit kde eutils

DESCRIPTION="Kuroo is a KDE Portage frontend"
HOMEPAGE="http://sourceforge.net/projects/kuroo"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"
LICENSE="GPL-2"

SLOT="3.5"
KEYWORDS="~x86 ~ppc ~sparc ~amd64"
IUSE=""

RDEPEND="app-portage/gentoolkit
		kde-misc/kdiff3
		dev-db/sqlite
		kde-base/kdesu"

need-kde 3.5
