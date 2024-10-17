#include "search_line.h"

#include "mainwidget.h"
class MainWidget;


Search_line::Search_line(QWidget* parent) : QLineEdit(parent) {
  // ui.setupUi(this);

  //  //todo 当输入框失去焦点的时候，且父级stacked_widget还在应用图标的第一页，则说明鼠标在外部有了点击事件，隐藏主窗口
  // connect(this, &QLineEdit::editingFinished, [&]() {
  // 	MainWidget* mainWidget = qobject_cast<MainWidget*>(this->parentWidget());
  // 	if (mainWidget != nullptr) {
  // 		if (mainWidget->stacked_widget->currentIndex() == 0) {
  // 			mainWidget->hide();
  // 		}
  // 	}
  // 	});
}

Search_line::~Search_line() {}

//焦点转移
void Search_line::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Down) {
    clearFocus();
    auto mainWidget = qobject_cast<MainWidget*>(this->parentWidget());
    if (mainWidget != nullptr) {
      mainWidget->sig_moveFocus(mainWidget->search_inner_widget);
    }
  }

  QLineEdit::keyPressEvent(event);
}
