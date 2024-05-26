#include "my_lineedit_exe_path.h"
#include <QDragEnterEvent>
#include <QMimeData>

my_line_edit_exe_path::my_line_edit_exe_path(QWidget* parent): QLineEdit(parent) {
  this->installEventFilter(this);
}

my_line_edit_exe_path::~my_line_edit_exe_path() {}

//拖放操作进入窗口部件的边界时被调用
void my_line_edit_exe_path::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction(); //接受该动作
  }
  else {
    event->ignore();
  }
  QLineEdit::dragEnterEvent(event);
}

//拖放操作完成，数据被放置在窗口部件上时被调用
void my_line_edit_exe_path::dropEvent(QDropEvent* event) {
  qInfo() << "获取拖拽文件路径";
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

bool my_line_edit_exe_path::eventFilter(QObject* watched, QEvent* event) {
  // 处理双击事件
  if (event->type() == QEvent::MouseButtonDblClick) {
    // 打开文件选择对话框
    qInfo() << "双击选择文件";
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("choose file"), QDir::homePath() + "/Desktop", "");
    if (!fileName.isEmpty()) {
      this->setText(fileName);
    }
  }

  return QLineEdit::eventFilter(watched, event);
}
