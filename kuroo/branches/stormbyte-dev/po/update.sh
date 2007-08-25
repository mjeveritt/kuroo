#! /bin/bash
echo "Updating POT file..."
make -f ../admin/Makefile.common package-messages &> /dev/null
for i in $( ls *.po ); do
	echo "Updating translation file: $i";
	msgmerge -U $i kuroo.pot &> /dev/null;
done

