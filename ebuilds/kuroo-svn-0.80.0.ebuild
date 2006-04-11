# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde subversion

ESVN_PROJECT="${PN/-svn}"
ESVN_REPO_URI="svn://kuroo.org/repos/kuroo/branches/0.80.0"
ESVN_STORE_DIR="${DISTDIR}/svn-src"
ESVN_BOOTSTRAP="make -f Makefile.cvs"

DESCRIPTION="A KDE Portage frontend"
HOMEPAGE="http://kuroo.org"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="~x86 ~amd64 ~ppc ~sparc"
IUSE="debug"

RDEPEND="!app-portage/guitoo
	|| (kde-misc/kdiff3 kde-base/kompare kde-base/kdesdk)"

need-kde 3.4
