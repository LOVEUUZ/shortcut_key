#include "my_dialog_accept_filepath.h"

#include <QHBoxLayout>
#include <filesystem>
#include <QPushButton>

My_dialog_accept_filePath::My_dialog_accept_filePath(QWidget* parent) {
  setFixedSize(400, 100);
  setModal(true);

  QHBoxLayout* layout   = new QHBoxLayout(this);
  btn_ok                = new QPushButton("确定", this);
  my_lineEdit_exe_path_ = new My_lineEdit_exe_path(this);
  my_lineEdit_exe_path_->setFocus();
  layout->addWidget(my_lineEdit_exe_path_, 5);
  layout->addWidget(btn_ok, 1);


  QObject::connect(btn_ok, &QPushButton::clicked, [&]() {
    QString     filePath     = my_lineEdit_exe_path_->text();
    std::string filePath_std = filePath.toLocal8Bit().constData();
    qDebug() << "File Path:" << filePath;
    if (filePath_std.empty()) {
      my_lineEdit_exe_path_->setText(tr("请填入正确文件路径"));
      return;
    }
    if (!std::filesystem::exists(filePath_std)) {
      my_lineEdit_exe_path_->setText(tr("请填入正确文件路径"));
      return;
    }


    // if (delegate == nullptr) return;

    emit str_path_ok();
    close();
    return;

    // this->accept();
  });
}


My_dialog_accept_filePath::~My_dialog_accept_filePath() {
  qDebug() << "My_dialog_accept_filePath 析构";
}


void My_dialog_accept_filePath::closeEvent(QCloseEvent* event) {
  // 安排在事件循环的安全点删除对象
  this->deleteLater();
  QDialog::closeEvent(event);
}
