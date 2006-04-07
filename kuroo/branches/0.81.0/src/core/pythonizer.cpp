/***************************************************************************
 *   Copyright (C) 2006 by Karim Ryde   *
 *   karim@bredband.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "pythonizer.h"

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#include <Python.h>

#include <qstringlist.h>
#include <qfile.h>

#include <kdebug.h>

#define foreach( x ) \
for( QStringList::ConstIterator it = x.begin(), end = x.end(); it != end; ++it )

#define LINE_INFO " ( " << k_funcinfo << "Line: " << __LINE__ << " )" << endl
#define DEBUG_LINE_INFO kdDebug() << LINE_INFO

Pythonizer::Pythonizer( const char *file, char *pyClass )
	: m_file( file ), m_pyClass( pyClass )
{
	kdDebug() << "Pythonizer::Pythonizer" << endl;
}

Pythonizer::~Pythonizer()
{
	kdDebug() << "Pythonizer::~Pythonizer" << endl;
}

QStringList Pythonizer::getPackages( char *pyModule )
{
	PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs, *pClass, *pInstance;
	
	// Initialize the Python Interpreter
	Py_Initialize();
	
    // Build the name object
	pName = PyString_FromString( m_file );
	
    // Load the module object
	pModule = PyImport_Import( pName );
	
    // pDict is a borrowed reference 
	pDict = PyModule_GetDict( pModule );
	
	// Build the name of a callable class 
	pClass = PyDict_GetItemString( pDict, m_pyClass );
	
    // Create an instance of the class
	if ( PyCallable_Check( pClass ) )
		pInstance = PyObject_CallObject( pClass, NULL );
	else {
		kdDebug() << "Can not create python class instance." << endl;
		return QStringList::QStringList();
	}
	
	// Call a method of the class with two parameters
	pValue = PyObject_CallMethod( pInstance, pyModule, NULL );
	
	if ( PyErr_Occurred() ) {
		PyErr_Print();
		return QStringList::QStringList();
	}
	
	PyObject *iterator = PyObject_GetIter( pValue );
	if ( iterator == NULL ) {
		PyErr_Print();
		return QStringList::QStringList();
	}
	
	QStringList packageList;
	PyObject *item;
	while ( item = PyIter_Next( iterator ) ) {
		packageList << PyString_AsString( item );
		
    	/* release reference when done */
		Py_DECREF( item );
	}
	Py_DECREF( iterator );
	
    // Clean up
	Py_DECREF( pModule );
	Py_DECREF( pName );
	
	// Finish the Python Interpreter
	Py_Finalize();
	
	return packageList;
}
