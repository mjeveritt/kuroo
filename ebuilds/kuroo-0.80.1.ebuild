# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde

DESCRIPTION="A KDE Portage frontend that allows you to do most common software maintainance tasks on gentoo systems"
HOMEPAGE="http://kuroo.org"
SRC_URI="http://files.kuroo.org/files/${P}.tar.bz2"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~ppc ~sparc ~amd64"
IUSE="debug"

RDEPEND="!app-portage/guitoo
	|| (kde-misc/kdiff3 kde-base/kompare kde-base/kdesdk)"

need-kde 3.4
