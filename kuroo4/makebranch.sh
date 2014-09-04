#!/bin/bash

E_WRONGARGS=85
if [ -n "$1" ]
then
	branch_name=kuroo4-$1
else
	echo "Usage: `makebranch.sh $0` name-of-branch"
	exit $E_WRONGARGS
fi

svn cp https://svn.code.sf.net/p/kuroo/code/kuroo4/trunk https://svn.code.sf.net/p/kuroo/code/kuroo4/tags/${branch_name}

cd tags

svn up ${branch_name}

cd ${branch_name}/src

sed -e "s/static const char version\[\] = \"kuroo-9999\";/static const char version\[\] = \"${branch_name}\";/g" -i main.cpp

svn ci main.cpp -m "Making a new release for ${branch_name}"

cd ../..

tar cvzf ../${branch_name}.tar.gz ${branch_name}
