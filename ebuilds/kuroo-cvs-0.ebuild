# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/app-portage/kuroo-cvs/kuroo-cvs-0.ebuild,v 1.2 2005/09/18 16:00:01 swegener Exp $

inherit kde cvs

ECVS_SERVER="cvs.sourceforge.net:/cvsroot/guitoo"
ECVS_MODULE="kuroo"
ECVS_TOP_DIR="${DISTDIR}/cvs-src/${PN}"
S=${WORKDIR}/${ECVS_MODULE}

DESCRIPTION="A KDE Portage frontend"
HOMEPAGE="http://guitoo.sourceforge.net"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~amd64 ~ppc ~sparc"
IUSE=""

RDEPEND="app-portage/gentoolkit
	!app-portage/guitoo
	!app-portage/kuroo
	|| (kde-base/dcoprss kde-base/kdenetwork)"

need-kde 3.2
