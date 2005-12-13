
#ifndef EMERGEINSPECTOR_H
#define EMERGEINSPECTOR_H

#include "emergeoptions.h"

#include <kdialogbase.h>

class EmergeInspector : public KDialogBase
{
Q_OBJECT
public:
    EmergeInspector( QWidget *parent = 0, const char *name = 0 );
    ~EmergeInspector();

	void				edit();
	QString				getOptions();

public slots:
	void				slotOk();
	
private slots:
	void				slotUser1();
	
private:
	EmergeOptionsBase*	dialog;
};

#endif
