#include "portage.h"
#include "scan.h"

void Job::setStatus( QString msg ) {
    emit status( msg );
}
void Job::setDone() {
    setProgress( total );
    setStatus( i18n("Done.") );
}

void Job::newProgress( uint totalSteps ) {
    count = 0;
    total = totalSteps;
}
void Job::setProgress( uint steps ) {
    count=steps;
    emit progress( (100*count) / total );
}
void Job::incProgress() {
    if( ( ++count % (total/100) ) == 0 ) setProgress( count );
}


Query::Query() : QSqlQuery() {}
Query::Query( QString queryString ) : QSqlQuery(queryString) {
    check();
    first();
}
bool Query::check() {
    if( !isActive() ) { kWarning() << lastQuery() << lastError(); return false; }
    return true;
}
QString Query::expand( QString format ) {
    for( int i=0; i<record().count(); ++i ) {
        format.replace("$"+record().fieldName(i), value(i).toString() );
    }
    return format;
}
QString Query::escape( QString raw ) {
    return raw.replace('\'', "''").replace('%', "&#37;");
}


bool versionOlderThan( const QString& a, const QString& b ) { return KStringHandler::naturalCompare( a,b ) < 0; }
bool versionNewerThan( const QString& a, const QString& b ) { return KStringHandler::naturalCompare( a,b ) >= 0; }
VersionList::VersionList() : QStringList() {}
VersionList::VersionList( const QStringList& other ) : QStringList( other ) {}
void VersionList::sortOldestFirst() { qSort( begin(), end(), versionOlderThan ); }
void VersionList::sortNewestFirst() { qSort( begin(), end(), versionNewerThan ); }
QString VersionList::toHTML() {
    QString html;
    foreach( QString version, VersionList(*this) ) {
        if( version.startsWith("i") ) html+="<b>"+version.remove(0,1)+"</b> "; //installed
        else if( version.startsWith("mi") ) html+="<font color=darkRed><b>"+version.remove(0,2)+"</b></font> "; //installed+masked
        else if( version.startsWith("m") ) html+="<font color=darkRed>"+version.remove(0,1)+"</font> "; //masked
        else html+=version+" ";
    }
    return html;
}
QString VersionList::toPlainText() { //QTableView doesnt accept rich-text, it would need a custom delegate
    QString html;
    foreach( QString version, VersionList(*this) ) {
        if( version.startsWith("i") ) html+="["+version.remove(0,1)+"] "; //installed
        else if( version.startsWith("mi") ) html+=version.remove(0,2)+" "; //installed+masked
        else if( version.startsWith("m") ) html+=version.remove(0,1)+" "; //masked
        else html+=version+" ";
    }
    return html;
}


int CategoryModel::rowCount(const QModelIndex& /*parent*/ ) const { return categories.size(); }
void CategoryModel::selectCategory( QModelIndex current ) { filter=current.data().toString(); refresh(); }
void CategoryModel::refresh() {
    //TODO: async
    categories.clear();
    categories << "All";
    QStringList pathList("/usr/portage"); //TODO: layman support
    foreach( QString path, pathList ) {
        QDir dCategory;
        dCategory.setNameFilters( QStringList("*-*") ); //<<"virtual"
        dCategory.setFilter( QDir::Dirs | QDir::NoSymLinks );
        dCategory.setSorting( QDir::Name );
        dCategory.cd( path );

        // Get list of categories in Portage
        foreach( QString categoryFolder, dCategory.entryList() ) {
            QString category = categoryFolder.split("-")[0];
            QString subcategory = categoryFolder.split("-")[1];
            if( filter.isEmpty() ) categories << category;
            else if( category==filter ) categories << subcategory;
        }
    }
    categories.removeDuplicates();
    reset();
}
QVariant CategoryModel::data(const QModelIndex& index, int role ) const {
    if(!index.isValid()) return QVariant();
    if(index.row() >= categories.size()) return QVariant();
    if(role == Qt::DisplayRole) return categories.at(index.row());
    else return QVariant();
}

PackageModel::PackageModel() : QAbstractTableModel(), query( new Query() ) {}
void PackageModel::selectCategory( QModelIndex current ) {
    category = current.data().toString();
    subcategory="All";
    refresh();
}
void PackageModel::selectSubcategory( QModelIndex current ) {
    subcategory = current.data().toString();
    refresh();
}
void PackageModel::selectFilter( QString filter ) {
    search = filter;
    refresh();
}
void PackageModel::selectAll() { status=""; refresh(); }
void PackageModel::selectAvailable() { status="available"; refresh(); }
void PackageModel::selectInstalled() { status="installed"; refresh(); }
void PackageModel::selectWorld() { status="world"; refresh(); }
void PackageModel::selectUpdate() { status="update"; refresh(); }
void PackageModel::refresh() {
    QString select("SELECT rowid,status,name,description,versions FROM package");
    QStringList match;
#ifdef FTS3
    if( !category.isEmpty() && category != "All" ) match << ("category:"+category);
    if( !subcategory.isEmpty() && subcategory != "All" ) match << ("subcategory:"+subcategory);
    //match += search;
    if( !match.isEmpty() ) select += "WHERE package MATCH '"+match.join(" ")+"'"
#else
    if( !category.isEmpty() && category != "All" ) match << ("category='"+category+"'");
    if( !subcategory.isEmpty() && subcategory != "All" ) match << ("subcategory='"+subcategory+"'");
    if( !status.isEmpty() ) match << ("status LIKE '%"+status+"%'");
    if( !search.isEmpty() ) {
        match += "((name LIKE '%"+search.split(" ").join("' AND name LIKE '%")+"%') OR"
                 "(description LIKE '%"+search.split(" ").join("' AND description LIKE '%")+"%'))";
    }
    if( !match.isEmpty() ) select += " WHERE "+match.join(" AND ");
#endif
    query->exec( select ); query->check();
    reset();
}
int PackageModel::rowCount(const QModelIndex& /*index*/) const { if( !query->last() ) return 0; return query->at()+1; }
int PackageModel::columnCount(const QModelIndex& /*index*/) const { return 4; }
QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        if( section==0 ) return KIcon("kuroo-package");
        else if( section==1 ) return i18n("Package (%1)").arg(rowCount());
        else if( section==2 ) return i18n("Description");
        else if( section==3 ) return i18n("Versions");
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}
QString statusToIcon( QString status ) {
    if( status.contains("queue") ) return "kuroo-queue";
    if( status.contains("update") ) return "kuroo-update";
    if( status.contains("world") ) return "kuroo-world";
    if( status.contains("installed") ) return "kuroo-stable";
    if( status.contains("available") ) return "kuroo-package";
    return "";
}
QVariant PackageModel::data ( const QModelIndex& index, int role ) const {
    query->check();
    query->seek( index.row() );
    if(role == Qt::UserRole) return query->value( 0 ); //UserRole is rowid for all column
    QSqlField field( query->record().field( index.column()+1 ) );
    if( field.name() == "status" ) {
        if(role == Qt::DisplayRole) return "";
        if(role == Qt::DecorationRole) return QVariant( KIcon( statusToIcon( field.value().toString() ) ) );
    }
    if( field.name() == "versions" ) {
        if(role == Qt::DisplayRole) return VersionList( field.value().toString().split(" ") ).toPlainText();
    }
    if(role == Qt::DisplayRole ) {
        return field.value();
    }
    return QVariant();
}


HistoryModel::HistoryModel() : QAbstractTableModel(), query( new Query() ) {}
int HistoryModel::columnCount(const QModelIndex& /*index*/) const { return 4; }
int HistoryModel::rowCount(const QModelIndex& /*index*/) const { if( !query->last() ) return 0; return query->at()+1; }
QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        if( section==0 ) return KIcon("kuroo-package");
        else if( section==1 ) return i18n("Package (%1)").arg(rowCount());
        else if( section==2 ) return i18n("Date");
        else if( section==3 ) return i18n("Duration");
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}
void HistoryModel::selectUnmerge( bool unmerge ) { if( unmerge ) action=""; else action="merge"; refresh(); }
void HistoryModel::selectFilter( QString filter ) {
    search=filter;
    refresh();
}
void HistoryModel::refresh() {
    QString select("SELECT rowid,action,package,timestamp,duration FROM history");
    QStringList match;
    if( !action.isEmpty() ) match << "action='"+action+"'";
    if( !search.isEmpty() ) match += "((package LIKE '%"+search.split(" ").join("' AND package LIKE '%")+"%')";
    if( !match.isEmpty() ) select += " WHERE "+match.join(" AND ");
    query->exec( select ); query->check();
    reset(); //resize columns
}
QVariant HistoryModel::data ( const QModelIndex& index, int role ) const {
    query->check();
    query->seek( index.row() );
    if(role == Qt::UserRole) return query->value( 0 ); //UserRole is rowid for all column
    QSqlField field( query->record().field( index.column()+1 ) );
    if( field.name() == "action" ) {
        if(role == Qt::DisplayRole) return "";
        if(role == Qt::DecorationRole) return QVariant( KIcon("kuroo-"+field.value().toString() ) );
    }
    if( field.name() == "timestamp" ) {
        if(role == Qt::DisplayRole) {
            QDateTime dt; dt.setTime_t( field.value().toInt() );
            return dt.toString("hh:mm, dddd d MMMM yyyy");
        }
    }
    if( field.name() == "duration" ) {
        if(role == Qt::DisplayRole) {
            QTime time(0,0, field.value().toInt() );
            return time.toString("h:mm:ss");
        }
    }
    if(role == Qt::DisplayRole ) {
        return field.value();
    }
    return QVariant();
}

QueueModel::QueueModel() : QAbstractTableModel() {
    stdout.setDevice( &emerge );
    connect( &emerge, SIGNAL( readyReadStandardOutput() ), this, SLOT( read() ) );
    connect( &emerge, SIGNAL( finished(int, QProcess::ExitStatus) ), this, SLOT( scan() ) );
}
int QueueModel::columnCount(const QModelIndex& /*index*/) const { return 6; }
int QueueModel::rowCount(const QModelIndex& /*index*/) const { return table.size(); }
QVariant QueueModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        if( section==0 ) return KIcon("kuroo-package");
        else if( section==1 ) return i18n("Package (%1)").arg(rowCount());
        else if( section==2 ) return i18n("New Version");
        else if( section==3 ) return i18n("Old Version");
        else if( section==4 ) return i18n("Use Flags");
        else if( section==5 ) return i18n("Size");
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}
void QueueModel::queue( QString package ) { packages << package; refresh(); }
void QueueModel::remove( QString package ) { packages.removeAll(package); refresh(); }
void QueueModel::clear() { packages.clear(); table.clear(); }
void QueueModel::sync() { emerge.start("emerge",QStringList("--sync")); }
void QueueModel::update() { packages<<"world"; }
void QueueModel::apply() { emerge.start("emerge",packages<<"-vuD"); }
void QueueModel::refresh() { emerge.start("emerge",packages<<"-pvuD"<<"--color"<<"n"<<"--columns"<<"--nospinner"); }
void QueueModel::read() { emit log( stdout.readAll() ); }
void QueueModel::scan() {
    table.clear();
    QTextStream stream( &emerge );
    while( !stream.atEnd() ) {
        //action,package,newVersion,oldVersion,use,size
        QRegExp rx("\\[(?:ebuild|block)?[ ]*([^ ]*)[ ]*\\][ ]*([^ ]*)[ ]*\\[([^]]*)\\][ ]*(?:\\[(.*)\\])?[ ]*USE=\"(.*)\"[ ]*(.*)");
        if( rx.exactMatch( stream.readLine() ) ) {
            QStringList row = rx.capturedTexts();
            row.removeFirst();
            table << row;
        }
    }
    reset(); //resize columns
}
QVariant QueueModel::data ( const QModelIndex& index, int role ) const {
    QString value = table[index.row()][index.column()];
    /*if( index.column() == 0 ) { display icon for actions
    }*/
    if(role == Qt::DisplayRole ) return value;
    return QVariant();
}

#include "portage.moc"
