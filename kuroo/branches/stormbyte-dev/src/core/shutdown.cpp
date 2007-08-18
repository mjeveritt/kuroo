//
// C++ Implementation: shutdown
//
// Description: 
//
//
// Author: karye <karye@users.sourceforge.net>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "common.h"
#include "shutdown.h"

Shutdown::Shutdown(QObject *parent, const char *name)
 : QObject(parent, name)
{
	connect( EmergeSingleton::Instance(), SIGNAL(signalEmergeComplete()),
		this, SLOT(slotTurnOff( void)));
}


Shutdown::~Shutdown()
{
}

void
Shutdown::init(QObject *parent)
{
	m_parent = parent;
}

void
Shutdown::slotEnable( bool enable )
{
	m_enable = enable;
}

void
Shutdown::slotTurnOff( void )
{
	if(m_enable)
		system("poweroff");
}

#include "shutdown.moc"
