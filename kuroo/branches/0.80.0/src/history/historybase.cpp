#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '/home/karye/repository/kuroo-svn/kuroo/branches/0.80.0/src/history/historybase.ui'
**
** Created: Tue Nov 29 17:00:03 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "historybase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <klistviewsearchline.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "historylistview.h"

/*
 *  Constructs a HistoryBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
HistoryBase::HistoryBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "HistoryBase" );
    HistoryBaseLayout = new QGridLayout( this, 1, 1, 0, 0, "HistoryBaseLayout"); 

    historyView = new HistoryListView( this, "historyView" );
    historyView->setMinimumSize( QSize( 0, 0 ) );

    HistoryBaseLayout->addWidget( historyView, 1, 0 );

    layout3 = new QGridLayout( 0, 1, 1, 2, 6, "layout3"); 
    spacer3 = new QSpacerItem( 80, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addItem( spacer3, 0, 1 );

    kcfg_viewUnmerges = new QCheckBox( this, "kcfg_viewUnmerges" );
    kcfg_viewUnmerges->setChecked( TRUE );

    layout3->addWidget( kcfg_viewUnmerges, 0, 0 );

	searchLineHistory = new KListViewSearchLineWidget( historyView, this, "searchLineHistory" );

    layout3->addWidget( searchLineHistory, 0, 2 );

    HistoryBaseLayout->addLayout( layout3, 0, 0 );
    languageChange();
    resize( QSize(591, 448).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
HistoryBase::~HistoryBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void HistoryBase::languageChange()
{
    setCaption( tr2i18n( "Form3" ) );
    kcfg_viewUnmerges->setText( tr2i18n( "View &unmerges" ) );
    kcfg_viewUnmerges->setAccel( QKeySequence( tr2i18n( "Alt+U" ) ) );
}

#include "historybase.moc"
