#include "my_dialog_accept_filepath.h"

#include <QHBoxLayout>
#include <filesystem>
#include <QPushButton>

my_dialog_accept_file_path::my_dialog_accept_file_path(QWidget* parent) {
  resize(600, 100);
  setModal(true);

  QHBoxLayout* layout   = new QHBoxLayout(this);
  btn_ok                = new QPushButton(tr("OK"), this);
  my_lineEdit_exe_path_ = new my_line_edit_exe_path(this);
  my_lineEdit_exe_path_->setFocus();
  layout->addWidget(my_lineEdit_exe_path_, 5);
  layout->addWidget(btn_ok, 1);

  //设置提示文本
  my_lineEdit_exe_path_->setPlaceholderText(tr("Drag the file or folder, or double -click, or enter manually"));

  QObject::connect(btn_ok, &QPushButton::clicked, [&]() {
    QString     filePath     = my_lineEdit_exe_path_->text();
    std::string filePath_std = filePath.toLocal8Bit().constData();
#ifdef _DEBUG
    qDebug() << "File Path:" << filePath;
#endif
    if (filePath_std.empty() || !std::filesystem::exists(filePath_std)) {
      my_lineEdit_exe_path_->setText("");
      my_lineEdit_exe_path_->setPlaceholderText(tr("Please fill in the correct file path"));
      return;
    }

    emit str_path_ok();
    close();
    return;
  });
}

my_dialog_accept_file_path::~my_dialog_accept_file_path() {
#ifdef _DEBUG
  qDebug() << "My_dialog_accept_filePath 析构";
#endif
}

void my_dialog_accept_file_path::closeEvent(QCloseEvent* event) {
  // 安排在事件循环的安全点删除对象
  this->deleteLater();
  QDialog::closeEvent(event);
}
