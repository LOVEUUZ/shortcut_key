#include "search_line.h"

#include "mainwidget.h"
class MainWidget;


Search_line::Search_line(QWidget* parent) : QLineEdit(parent) {
	// ui.setupUi(this);


}

Search_line::~Search_line() {}

//焦点转移
void Search_line::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Down) {
		clearFocus();
		auto mainWidget = qobject_cast<MainWidget*>(this->parentWidget());
		if (mainWidget != nullptr) {
			mainWidget->sig_move_focus(mainWidget->search_inner_widget);
		}
	}

	QLineEdit::keyPressEvent(event);
}
