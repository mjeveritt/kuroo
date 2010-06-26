#include <QApplication>

#include "queuelistdelegate.h"
#include "queuelistitem.h"

#define MIN(x,y)	(x) > (y) ? (y) : (x)

void QueueListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QueueListItem *item = static_cast<QueueListItem*>(index.internalPointer());
	if (item && index.column() == 5 && item->hasStarted())
	{
		int progress;
		if (item->duration() > 0)
			progress = MIN(item->steps()/item->duration()*100, 100);
		else
			progress = item->steps();

	 	QStyleOptionProgressBar progressBarOption;
	 	progressBarOption.rect = option.rect;
	 	progressBarOption.minimum = 0;
		
		if (item->isComplete())
	 		progressBarOption.maximum = 100;
		else
			progressBarOption.maximum = item->duration() > 0 ? item->duration() : 0;
		
		if (item->isComplete())
	 		progressBarOption.progress = 100;
		else
	 		progressBarOption.progress = progress;
	 	progressBarOption.text = QString::number(progress) + " %";
		
		if (item->isComplete())
	 		progressBarOption.textVisible = true;
		else
	 		progressBarOption.textVisible = item->duration() > 0 ? true : false;
		
		QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

	}
	else
		QStyledItemDelegate::paint(painter, option, index);
}
