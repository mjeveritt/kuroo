#include "historylistview.h"
#include "historybase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdialog.h>
#include <klocale.h>
#include <klistviewsearchline.h>

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
	
	layout1 = new QGridLayout( 0, 1, 1, 0, 0, "layout1"); 
	line1 = new QFrame( this, "line1" );
	line1->setPaletteForegroundColor( QColor( 185, 185, 185 ) );
	line1->setFrameShape( QFrame::HLine );
	line1->setFrameShadow( QFrame::Plain );
	line1->setLineWidth( 1 );
	line1->setFrameShape( QFrame::HLine );
	line1->setMargin( 1 );
	layout1->addWidget( line1, 0, 0 );
    historyView = new HistoryListView( this, "historyView" );
    historyView->setMinimumSize( QSize( 0, 0 ) );
    layout1->addWidget( historyView, 1, 0 );
	HistoryBaseLayout->addLayout( layout1, 1, 0 );
	
    layout3 = new QGridLayout( 0, 1, 1, 2, 0, "layout3"); 
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
