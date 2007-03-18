/***************************************************************************
 *   Copyright (C) 2005 by Karye   *
 *   karye@users.sourceforge.net   *
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

#include "common.h"
#include "mainwindow.h"

#include <qobjectlist.h>

#include <kdebug.h>

/**
 * @class MainWindow
 * @short Subclassed KMainWindow to accomodate statusBar containing progressbar.
 */
MainWindow::MainWindow( QWidget *parent, const char *name )
	: KMainWindow( parent, name ), sb( 0 )
{
}

MainWindow::~MainWindow()
{
}

/**
 * Create statusBar.
 * @return pointer to statusBar
 */
KuroolitoStatusBar* MainWindow::statusBar()
{
	sb = new KuroolitoStatusBar( this );
	return sb;
}

#include "mainwindow.moc"