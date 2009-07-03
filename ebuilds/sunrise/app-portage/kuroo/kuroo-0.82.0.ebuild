# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header:
# /var/www/viewcvs.gentoo.org/raw_cvs/gentoo-x86/app-portage/kuroo/kuroo-0.82.0.ebuild,v 1.1 2009/06/26 20:43:16 cryos Exp $

inherit kde eutils

DESCRIPTION="Kuroo is a KDE Portage frontend"
HOMEPAGE="http://sourceforge.net/projects/kuroo"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.bz2"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~ppc ~sparc ~amd64"
IUSE=""

RDEPEND="app-portage/gentoolkit
		kde-misc/kdiff3
		dev-db/sqlite
		kde-base/kdesu"

need-kde 3.5
