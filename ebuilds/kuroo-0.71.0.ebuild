# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde

DESCRIPTION="A KDE Portage frontend"
HOMEPAGE="http://guitoo.sourceforge.net"
SRC_URI="mirror://sourceforge/guitoo/${P}.tar.bz2"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~amd64 ~ppc ~sparc"
IUSE=""
RDEPEND="!app-portage/guitoo
	app-portage/gentoolkit
	kde-misc/kdiff3
	|| (kde-base/dcoprss kde-base/kdenetwork)"

need-kde 3.2
