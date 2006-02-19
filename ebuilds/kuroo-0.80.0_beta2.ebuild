# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde

DESCRIPTION="A KDE Portage frontend"
HOMEPAGE="http://tux.myftp.org/kuroo"
SRC_URI="http://tux.myftp.org/kuroo/browser/packages/${P}.tar.bz2"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~ppc ~sparc ~amd64"
IUSE="debug"

RDEPEND="app-portage/gentoolkit
	!app-portage/guitoo
	|| (kde-misc/kdiff3 kde-base/kdesdk)
	|| (kde-base/dcoprss kde-base/kdenetwork)"

need-kde 3.4
