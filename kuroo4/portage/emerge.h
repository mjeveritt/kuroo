//Authors: Karye; Fauconneau
#ifndef EMERGE_H
#define EMERGE_H

#include <QObject>
#include <QStringList>
#include <QFile>

#include <KProcess>

#include "package.h"

/**
 * @class Emerge
 * @short The Gentoo emerge command.
 */
class Emerge : public QObject
{
    Q_OBJECT

    Emerge();
	~Emerge();

public:
	void						inputText( const QString& text );
	bool						stop();
	bool 						isRunning() const;

    const QRegExp rxEmerge();

    bool emerge( const QStringList& args, QString status, const QStringList& packageList=QStringList() );
	bool 						pretend( const QStringList& packageList );
	bool 						queue( const QStringList& packageList );
	bool 						unmerge( const QStringList& packageList );
	bool						quickpkg( const QStringList& packageList );
	bool						sync();
	bool						checkUpdates();

	/**
	 * Are we paused?
	 * @return bool
	 */
	inline bool					isPaused() const { return m_isPaused; }
	/**
	 * Can we pasue?
	 * @return bool
	 */
	inline bool					canPause() const { return m_pausable; }
	
	/**
	 * @return list of packages parsed out from emerge output.
	 */
	inline const 	EmergePackageList 		packageList() const { return m_emergePackageList; }
	const 	QString					packageMessage();
	/**
	 * Set Skip Housekeeping
	 */
	inline void					setSkipHousekeeping(const bool& x) { m_skipHousekeeping = x;}

public slots:
	void						slotPause();
	void						slotUnpause();
	
private:
    void startRevdepRebuild();
	bool						countEtcUpdates( const QString& line );
	void						askUnmaskPackage( const QString& packageKeyword );
	/**
	 * Do we Skip housekeeping?
	 * @return bool
	 */
	inline bool					skipHousekeeping() const { return m_skipHousekeeping; }
	
private slots:
    void 						slotDone();
    void 						slotEmergeOutput();
	void						slotTryEmerge();
    void						slotBackupComplete();
    void						slotEmergeDistfilesComplete();
    void						slotEClean2Complete();
    void						slotRevdepRebuildComplete();

	
signals:
	void						signalEmergeComplete();
	
private:
    KProcess*					eProc;
    KProcess*                   eClean1;
    KProcess*                   eClean2;
    KProcess*                   ioRevdepRebuild;
        
	// Used to collect ewarn and einfo messages spaning multiple lines
	bool						m_completedFlag;

	// Used to track a quickpkg backup
	bool						m_backupComplete;
	bool						m_backingUp;

	// Can we pause this eProc?
	bool						m_pausable;
	bool						m_isPaused;

	// should we be ecleaning?
	bool						m_doeclean;
        bool                                            m_dorevdeprebuild;
	
	// Package with the important message
	QString						m_importantMessagePackage;
	
	// Collects messages from emerge, like masked errors, ewarn and einfos
	QString 					m_importantMessage;
	
	// The current package
	QString						m_packageMessage;
	
	// The parsed package emerge says need unmasking
	QString						m_unmasked;
	
	// Collect all blocking packages
	QStringList 				m_blocks;
	
	// Remember packages emerge started with, used when auto-unmasking
	QStringList					m_lastEmergeList;
	
	// List of parsed packages
	EmergePackageList			m_emergePackageList;
	
	// Count of etc-updates files to merge
	int							m_etcUpdateCount;
        
        bool                                            m_skipHousekeeping;
};

#endif
