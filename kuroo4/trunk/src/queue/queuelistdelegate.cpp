#include <QApplication>

#include "queuelistdelegate.h"
#include "queuelistitem.h"

void QueueListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QueueListItem *item = static_cast<QueueListItem*>(index.internalPointer());
	if (item && index.column() == 5 && item->hasStarted())
	{
		//int progress = item->progress();
		int progress = item->steps();

	 	QStyleOptionProgressBar progressBarOption;
	 	progressBarOption.rect = option.rect;
	 	progressBarOption.minimum = 0;
	 	progressBarOption.maximum = item->duration();
	 	//progressBarOption.maximum = 100;
	 	progressBarOption.progress = progress;
	 	progressBarOption.text = QString::number(progress) + " %";
	 	progressBarOption.textVisible = true;
		
		QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

	}
	else
		QStyledItemDelegate::paint(painter, option, index);
}
