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

#ifndef VERSIONVIEW_H
#define VERSIONVIEW_H

#include <klistview.h>

class VersionView : public KListView
{
Q_OBJECT
public:
    VersionView( QWidget *parent = 0, const char *name = 0 );
    ~VersionView();

    /**
     * @class VersionViewItem
     * @short Subclass for formating text.
     */
    class 		VersionItem : public KListViewItem
    {
	    public:
		    VersionItem( QListView* parent, const char* version, const bool& isInstalled, const int& stability );
		    ~VersionItem();
	
		    inline bool	isInstalled() { return m_isInstalled; }
	
	    protected:
		    void 	paintCell( QPainter *p, const QColorGroup &cg, const int& column, const int& width, const int& alignment );
	
	    private:
		    bool	m_isInstalled;
		    int		m_stability;
    };
	
	void		insertItem( const char* version, const char* stability, const char* size, const bool& isInstalled );
	void		usedForInstallation( const QString& version );
	inline int	hasUpdate() const { return m_installedIndex - m_emergeIndex; }
	inline QString 	updateVersion() const { return m_emergeVersion; }
	
private:
	QString		m_emergeVersion;
	int			m_installedIndex, m_emergeIndex;
};

#endif
