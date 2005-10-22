# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde

DESCRIPTION="A KDE Portage frontend"
HOMEPAGE="http://guitoo.sourceforge.net"
SRC_URI="mirror://sourceforge/guitoo/${P}.tar.gz"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE=""
DEPEND="!app-admin/guitoo app-portage/gentoolkit || (kde-base/dcoprss kde-base/kdenetwork)"

need-kde 3.2
