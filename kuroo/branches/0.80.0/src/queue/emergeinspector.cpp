
#include "emergeinspector.h"
#include "common.h"

#include <qcheckbox.h>
#include <qradiobutton.h>

#include <klineedit.h>

EmergeInspector::EmergeInspector( QWidget *parent )
	: KDialogBase( KDialogBase::Swallow, i18n( "Emerge Options" ), KDialogBase::User1 | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent, i18n( "Save" ), false )
{
	setButtonText( KDialogBase::User1, i18n( "Clear" ) );
	dialog = new EmergeOptionsBase( this );
	setMainWidget( dialog );
}

EmergeInspector::~EmergeInspector()
{
}

/**
 * Open dialog for advanced emerge options.
 */
void EmergeInspector::edit()
{
	show();

	// Get options
	QString options( dialog->emergeOptionsText->text() );
	
	if ( !options.isEmpty() ) {
		const QStringList optionsList = QStringList::split( " ", options );
		foreach ( optionsList ) {
			options = ( *it ).stripWhiteSpace();
			
			if ( options == "--buildpkg" )
				dialog->radioBuildpkg->toggle();
			
			if ( options == "--buildpkgonly" )
				dialog->radioBuildpkgonly->toggle();
			
			if ( options == "--usepkg" )
				dialog->radioUsepkg->toggle();
			
			if ( options == "--usepkgonly" )
				dialog->radioUsepkgonly->toggle();
			
			if ( options == "--nodeps" )
				dialog->radioNodeps->toggle();
			
			if ( options == "--onlydeps" )
				dialog->radioOnlydeps->toggle();
			
			if ( options == "--verbose" )
				dialog->radioVerbose->toggle();
			
			if ( options == "--quiet" )
				dialog->radioQuiet->toggle();
			
			if ( options == "--pretend" )
				dialog->checkPretend->toggle();
			
			if ( options == "--deep" )
				dialog->checkDeep->toggle();
			
			if ( options == "--update" )
				dialog->checkUpdate->toggle();
			
			if ( options == "--upgradeonly" )
				dialog->checkUpgradeonly->toggle();
			
			if ( options == "--fetchonly" )
				dialog->checkFetchonly->toggle();
			
			if ( options == "--emptytree" )
				dialog->checkEmptytree->toggle();
			
			if ( options == "--debug" )
				dialog->checkDebug->toggle();
			
			if ( options == "--noconfmem" )
				dialog->checkNoconfmem->toggle();
			
			if ( options == "--oneshot" )
				dialog->checkOneshot->toggle();
			
			if ( options == "--noreplace" )
				dialog->checkNoreplace->toggle();
			
			if ( options == "--newuse" )
				dialog->checkNewUse->toggle();
		}
	}
}

/**
 * Write options back to lineedit
 */
void EmergeInspector::slotOk()
{
	QStringList optionsList;
	
	if ( dialog->radioBuildpkg->isOn() ) 
		optionsList += "--buildpkg";
	else
		if ( dialog->radioBuildpkgonly->isOn() )
			optionsList += "--buildpkgonly";
	
	if ( dialog->radioUsepkg->isOn() )
		optionsList += "--usepkg";
	else
		if ( dialog->radioUsepkgonly->isOn() )
			optionsList += "--usepkgonly";
	
	if ( dialog->radioNodeps->isOn() )
		optionsList += "--nodeps";
	else
		if ( dialog->radioOnlydeps->isOn() )
			optionsList += "--onlydeps";
	
	if ( dialog->radioVerbose->isOn() )
		optionsList += "--verbose";
	else
		if ( dialog->radioQuiet->isOn() )
			optionsList += "--quiet";
	
	if ( dialog->checkPretend->isChecked() )
		optionsList += "--pretend";
	
	if ( dialog->checkDeep->isChecked() )
		optionsList += "--deep";
	
	if ( dialog->checkUpdate->isChecked() )
		optionsList += "--update";
	
	if ( dialog->checkUpgradeonly->isChecked() )
		optionsList += "--upgradeonly";
	
	if ( dialog->checkFetchonly->isChecked() )
		optionsList += "--fetchonly";
	
	if ( dialog->checkEmptytree->isChecked() )
		optionsList += "--emptytree";
	
	if ( dialog->checkDebug->isChecked() ) 
		optionsList += "--debug";
	
	if ( dialog->checkNoconfmem->isChecked() )
		optionsList += "--noconfmem";
	
	if ( dialog->checkOneshot->isChecked() )
		optionsList += "--oneshot";
	
	if ( dialog->checkNoreplace->isChecked() )
		optionsList += "--noreplace";
	
	if ( dialog->checkNewUse->isChecked() )
		optionsList += "--newuse";
	
	dialog->emergeOptionsText->setText( optionsList.join( " " ) );
	close();
}

void EmergeInspector::slotUser1()
{
	if ( dialog->radioBuildpkg->isOn() )
		dialog->radioBuildpkg->toggle();
	
	if ( dialog->radioBuildpkgonly->isOn() ) 
		dialog->radioBuildpkgonly->toggle();
	
	if ( dialog->radioUsepkg->isOn() ) 
		dialog->radioUsepkg->toggle();
	
	if ( dialog->radioUsepkgonly->isOn() ) 
		dialog->radioUsepkgonly->toggle();
	
	if ( dialog->radioNodeps->isOn() ) 
		dialog->radioNodeps->toggle();
	
	if ( dialog->radioOnlydeps->isOn() ) 
		dialog->radioOnlydeps->toggle();
	
	if ( dialog->radioVerbose->isOn() ) 
		dialog->radioVerbose->toggle();
	
	if ( dialog->radioQuiet->isOn() ) 
		dialog->radioQuiet->toggle();
	
	dialog->checkPretend->setChecked( false );
	dialog->checkDeep->setChecked( false );
	dialog->checkUpdate->setChecked( false );
	dialog->checkUpgradeonly->setChecked( false );
	dialog->checkFetchonly->setChecked( false );
	dialog->checkEmptytree->setChecked( false );
	dialog->checkDebug->setChecked( false );
	dialog->checkNoconfmem->setChecked( false );
	dialog->checkOneshot->setChecked( false );
	dialog->checkNoreplace->setChecked( false );
	dialog->checkNewUse->setChecked( false );
	
	dialog->emergeOptionsText->clear();
}

QString EmergeInspector::getOptions()
{
	return dialog->emergeOptionsText->text();
}

#include "emergeinspector.moc"
