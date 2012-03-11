#include "scan.h"

/**
 * @class ScanPortageJob
 * @short scan /usr/portage, /var/db/pkg, /var/lib/portage/world, /var/log/emerge.log
 */
void ScanPortageJob::run()
{
    QSqlDatabase::database().transaction();
    setStatus( i18n("Scanning available packages...") );
    newProgress( 14000 );
    QSqlQuery dropPackage("DROP TABLE package;");
    QSqlQuery createPackage(
#ifdef FTS3
#error not supported
                     "CREATE VIRTUAL TABLE package USING fts3"
#else
                     "CREATE TABLE package "
#endif
                     "( category, subcategory, name, description, versions, status, duration )");
    //CLEAN: duplicated category folder walking code
    QStringList pathList("/usr/portage"); //TODO: layman support
    foreach( QString path, pathList ) {
        QDir dCategory;
        dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
        dCategory.setNameFilters( QStringList("*-*") ); //<<"virtual"
        dCategory.cd( path );
        foreach( QString categoryFolder, dCategory.entryList() ) {
            if( categoryFolder != "kde-base" ) continue;
            QString category = categoryFolder.split("-")[0];
            QString subcategory = categoryFolder.split("-")[1];

            QDir dPackage;
            dPackage.setFilter( QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
            dPackage.cd( path + "/" + categoryFolder);
            foreach( QString package, dPackage.entryList() ) {
                //get all available versions
                QDir dVersion;
                dVersion.setFilter( QDir::Files | QDir::NoSymLinks );
                dVersion.cd( path+"/"+categoryFolder+"/"+package );
                VersionList versions;
                foreach( QString file, dVersion.entryList() ) {
                    QRegExp rx( QRegExp::escape(package) + "-(.*).ebuild");
                    if( !rx.exactMatch(file) ) continue;
                    versions << rx.cap(1);
                }
                if( !versions.size() ) qWarning() << dVersion.entryList() << (package+"-(.*).ebuild");
                versions.sortNewestFirst();
                //read package info from newest ebuild
                QString description="none found";
                QFile ebuild( path+"/"+categoryFolder+"/"+package+"/"+package+"-"+versions.first()+".ebuild" );
                ebuild.open( QIODevice::ReadOnly );
                QTextStream stream( &ebuild );
                while ( !stream.atEnd() ) {
                    QString line = stream.readLine();
                    if( line.startsWith("DESCRIPTION=") ) {
                        description = Query::escape( line.section('"',1,1 ) );
                        break;
                    }
                    //FIXME: homepage/use/licenses/slot/keywords require advanced ebuild parsing
                }
                Query insert("INSERT INTO package (category,subcategory,name,description,versions,status)"
                             "VALUES ('"+category+"','"+subcategory+"','"+package+"','"+description+
                             "','"+versions.join(" ")+"','available')");
                incProgress();
            }
        }
    }
    setStatus( i18n("Scanning installed packages...") );
    newProgress( 1000 );
    //CLEAN: duplicated category folder walking code
    QString path("/var/db/pkg/");
    QDir dCategory;
    dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
    dCategory.setNameFilters( QStringList("*-*") ); //<<"virtual"
    dCategory.cd( path );
    foreach( QString categoryFolder, dCategory.entryList() ) {
        QString category = categoryFolder.split("-")[0];
        QString subcategory = categoryFolder.split("-")[1];

        QDir dPackage;
        dPackage.setFilter( QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
        dPackage.cd( path + "/" + categoryFolder);
        foreach( QString packageFolder, dPackage.entryList() ) {
            QRegExp rx("([^.-]*(?:-[^.-]*[^.-\\d]+[^.-]*)*)-(.*)"); //[name(-name)*]-[version]
            rx.exactMatch(packageFolder);
            QString package = rx.cap(1);
            QString version = rx.cap(2);
            Query select("SELECT versions FROM package "
                         "WHERE category='"+category+"' AND subcategory='"+subcategory+"' AND name='"+package+"'");
            if( !select.first() ) {
                //qWarning() << "not in package database: "+category+"-"+subcategory+"/"+package;
            } else {
                //TODO: read SIZE
                QString versions = select.value(0).toString();
                versions.insert( versions.indexOf( version ), "i" );
                Query update("UPDATE package SET versions='"+versions+"',status=status||' installed' "
                             "WHERE category='"+category+"' AND subcategory='"+subcategory+"' AND name='"+package+"'");
            }
            incProgress();
        }
    }
    setStatus( i18n("Scanning world packages...") );
    QFile world("/var/lib/portage/world");
    world.open( QIODevice::ReadOnly );
    QTextStream worldStream( &world );
    while ( !worldStream.atEnd() ) {
        QString line = worldStream.readLine();
        QRegExp rx("(.*)-(.*)/(.*)");
        rx.exactMatch( line );
        QString category = rx.cap(1);
        QString subcategory = rx.cap(2);
        QString package = rx.cap(3);
        Query update("UPDATE package SET status=status||' world' "
                     "WHERE category='"+category+"' AND subcategory='"+subcategory+"' AND name='"+package+"'");
        incProgress();
    }
    setStatus( i18n("Scanning emerge history...") );
    QSqlQuery dropHistory("DROP TABLE history");
    QSqlQuery createHistory("CREATE TABLE history ( package, timestamp, duration, action )");
    QFile log("/var/log/emerge.log");
    log.open( QIODevice::ReadOnly );
    QTextStream logStream( &log );
    QMap<QString, int> emergeStart;
    while( !logStream.atEnd() ) {
        QString line = logStream.readLine();
        QRegExp rx("(\\d+): (.*) ([^ ]*)-(.*)/([^.-]*(?:-[^.-]*[^.-\\d]+[^.-]*)*)-([^: ]*)");
        rx.exactMatch(line);
        QString timeStamp = rx.cap(1);
        QString emergeLine = rx.cap(2);
        QString category = rx.cap(3);
        QString subcategory = rx.cap(4);
        QString name = rx.cap(5);
        QString version = rx.cap(6);
        QString package = category+"-"+subcategory+"/"+name+"-"+version;

        if ( emergeLine.contains( ">>> emerge" ) ) {
            emergeStart[package] = timeStamp.toInt();
        } else if ( emergeLine.contains( "::: completed emerge " ) ) {
            int duration = timeStamp.toInt()-emergeStart[package];
            QSqlQuery("INSERT INTO history (package,timestamp,duration,action) "
                      "VALUES ('"+package+"','"+timeStamp+"','"+QString::number(duration)+"','merge')");
            QSqlQuery("UPDATE package SET duration='"+QString::number(duration)+"' "
                      "WHERE category='"+category+"' AND subcategory='"+subcategory+"' AND name='"+name+"'");
        } else if ( emergeLine.contains( ">>> unmerge success" ) ) {
            int duration = timeStamp.toInt()-emergeStart[package];
            QSqlQuery("INSERT INTO history (package,timestamp,duration,action) "
                      "VALUES ('"+package+"','"+timeStamp+"','"+QString::number(duration)+"','unmerge')");
        }
        incProgress();
    }
    QSqlDatabase::database().commit();
    setDone();
}

#include "scan.moc"
