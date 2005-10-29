/***************************************************************************
*   Copyright (C) 2004 by karye                                           *
*   karye@users.sourceforge.net                                           *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "diskusage.h"
#include "common.h"

#include <kdirsize.h>

/**
 * Scan size of Portage directories with KIO::KDirSize synchronously.
 */
DiskUsage::DiskUsage( QObject *parent )
	: QObject( parent )
{
}

DiskUsage::~DiskUsage()
{
}

void DiskUsage::init( QObject *myParent )
{
	parent = myParent;
}

/**
 * Get portage directory size scan @fixme: how to abort!
 * @param path	directory to scan
 * @return size in MB
 */
QString DiskUsage::scanSize( const QString& path )
{
	return QString::number((long)( KDirSize::dirSize(KURL::KURL( path ))/1048576)) + " MB";
}

/**
 * Get portage directory size scan @fixme: how to abort!
 * @param path	directory to scan
 * @return size in MB
 */
long DiskUsage::scanSizeLong( const QString& path )
{
	return (long)( KDirSize::dirSize(KURL::KURL( path ))/1048576);
}

#include "diskusage.moc"
