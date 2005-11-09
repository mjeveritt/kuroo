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
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <kstatusbar.h>
#include <kprogress.h>
#include <qobject.h>

class QTimer;

/**
 * @class KurooStatusBar
 * @short Singleton statusbar with progressbar in right corner.
 */
class KurooStatusBar : public KStatusBar
{
Q_OBJECT
	static KurooStatusBar* s_instance;
	
public:
	KurooStatusBar( QWidget *parent = 0 );
    ~KurooStatusBar();
	
	static 			KurooStatusBar* instance() { return s_instance; }
	
	/**
 	 * Progress methods.
 	 */
	void 			setProgressStatus( const QString& text );
	void			setTotalSteps( int total );
	void 			setProgress( int steps );
	void			startTimer();
	void			stopTimer();
	void			startProgress();
	
public slots:
	void			oneStep();
	void			advance();

private:
	KProgress 		*statusBarProgress;
	QLabel 			*statusBarLabel;
	QTimer 			*internalTimer, *diffTimer;
	int				timerSteps;
};

#endif
