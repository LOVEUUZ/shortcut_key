#include "my_lineedit_exe_path.h"
#include <QDragEnterEvent>
#include <QMimeData>

My_lineEdit_exe_path::My_lineEdit_exe_path(QWidget* parent)
  : QLineEdit(parent) {}

My_lineEdit_exe_path::~My_lineEdit_exe_path() {}

//拖放操作进入窗口部件的边界时被调用
void My_lineEdit_exe_path::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction(); //接受该动作
  }
  else {
    event->ignore();
  }
  QLineEdit::dragEnterEvent(event);
}

//拖放操作完成，数据被放置在窗口部件上时被调用
void My_lineEdit_exe_path::dropEvent(QDropEvent* event) {
  const QMimeData* mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    QList<QUrl> urlList = mimeData->urls();
    if (urlList.size() == 1) {
      // 获取文件的路径并显示在LineEdit中
      QString filePath = urlList.first().toLocalFile();
      // 设置文本
      QLineEdit::setText(filePath);
    }
    event->acceptProposedAction();
  }
  else {
    event->ignore();
  }

  // 不能调用父类的dropEvent方法
  // QLineEdit::dropEvent(event);
}
