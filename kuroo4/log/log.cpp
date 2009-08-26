#include "log.h"

Log::Log( QWidget* parent ) : QWidget( parent ) {
    setupUi( this );
}

void Log::append( QString msg ) { logEdit->appendPlainText( msg ); }

#include "log.moc"
