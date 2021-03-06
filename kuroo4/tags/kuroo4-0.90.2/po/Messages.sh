#!/bin/sh
#This is all copied from http://techbase.kde.org/Development/Tutorials/Localization/i18n_Build_Systems
BASEDIR="../src/"	# root of translatable sources
PROJECT="kuroo"		# project name
BUGADDR="http://sourceforge.net/tracker/?group_id=2040813&atid=1114070"	# MSGID-Bugs
WDIR=`pwd`

echo "Preparing rc files"
cd ${BASEDIR}
find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' > ${WDIR}/rcfiles.list
xargs --arg-file=${WDIR}/rcfiles.list extractrc > ${WDIR}/rc.cpp
# additional strings for KAboutData
echo 'i18nc("NAME OF TRANSLATORS", "Your names");' >> ${WDIR}/rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS", "Your emails");' >> ${WDIR}/rc.cpp
cd ${WDIR}
echo "Done preparing rc files"

echo "Extracting messages"
cd ${BASEDIR}
#find . -name '*.cpp' -o -name '*.h' -o -name '*.c' > ${WDIR}/infiles.list
find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C > ${WDIR}/infiles.list
echo "rc.cpp" >> ${WDIR}/infiles.list
cd ${WDIR}
xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:lc,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 --files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot || { echo "error while calling xgettext. aborting"; exit 1; }
#-msgid-bugs-address="${BUGADDR}"
echo "Done extracting messages"

echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
	echo $cat
	msgmerge -o $cat.new $cat ${PROJECT}.pot
	mv $cat.new $cat
done
echo "Done merging translations"

echo "Cleaning up"
cd ${WDIR}
rm rcfiles.list
rm infiles.list
rm rc.cpp
echo "Done"
