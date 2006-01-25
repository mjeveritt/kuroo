# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

inherit kde subversion

ESVN_PROJECT="${PN/-svn}"
ESVN_REPO_URI="svn://tux.myftp.org/repos/kuroo/tags/0.80.0_alfa2"
ESVN_STORE_DIR="${DISTDIR}/svn-src"
ESVN_BOOTSTRAP="make -f Makefile.cvs"

DESCRIPTION="A KDE Portage frontend"
HOMEPAGE="http://tux.myftp.org/kuroo"
LICENSE="GPL-2"

SLOT="0"
KEYWORDS="x86 amd64"
IUSE="debug"

RDEPEND="app-portage/gentoolkit
	!app-portage/guitoo
	|| (kde-misc/kdiff3 kde-base/kdesdk)
	|| (kde-base/dcoprss kde-base/kdenetwork)"

need-kde 3.2
