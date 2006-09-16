//
// C++ Interface: shutdown
//
// Description: Shutdown the computer
//
//
// Author: babali <bique.alexandre@gmail.com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <qobject.h>

/**
	@author babali <bique.alexandre@gmail.com>
*/
class Shutdown : public QObject
{
Q_OBJECT
public:
    Shutdown(QObject *parent = 0, const char *name = 0);

    ~Shutdown();
    
    void 		init( QObject *parent = 0 );

public slots:
	void		slotEnable(bool enable);
	void		slotTurnOff(void);

private:
	QObject		*m_parent;
	bool		m_enable;
};

#endif
