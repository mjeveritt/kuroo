#!/bin/bash

E_WRONGARGS=85
if [ -n "$1" ]
then
	branch_name=kuroo4-$1
else
	echo "Usage: `makebranch.sh $0` name-of-branch"
	exit $E_WRONGARGS
fi

svn cp https://kuroo.svn.sourceforge.net/svnroot/kuroo/kuroo4/trunk https://kuroo.svn.sourceforge.net/svnroot/kuroo/kuroo4/tags/${branch_name}

cd tags

svn up ${branch_name}

cd ${branch_name}/src

sed -e "s/static const char version\[\] = \"kuroo-9999\";/static const char version\[\] = \"${branch_name}\";/g" -i main.cpp

svn ci main.cpp -m "Making a new release for ${branch_name}"


