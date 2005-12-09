
#ifndef PRETENDINSPECTOR_H
#define PRETENDINSPECTOR_H

#include "resultsbase.h"

#include <kdialogbase.h>

class PretendInspector : public KDialogBase
{
Q_OBJECT
public:
    PretendInspector( QWidget *parent = 0, const char *name = 0 );
    ~PretendInspector();

public slots:
	void				showResults();
	
private slots:
	void				slotOk();
	
private:
	EmergePretendBase*	dialog;
};

#endif
