# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde

DESCRIPTION="Kuroo is a KDE Portage frontend."
HOMEPAGE="http://kuroo.org/"
SRC_URI="http://files.kuroo.org/${P}.tar.bz2"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~ppc ~sparc ~amd64"
IUSE=""

RDEPEND="app-portage/gentoolkit
	kde-misc/kdiff3
	|| ( kde-base/kdesu kde-base/kdebase )"
need-kde 3.5