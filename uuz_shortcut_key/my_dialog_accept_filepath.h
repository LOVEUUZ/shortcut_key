#pragma once

#include <QDialog>
#include <QPushButton>
#include "my_lineedit_exe_path.h"

//用于接收启动项的弹窗，通过委托调用
class my_dialog_accept_file_path : public QDialog {
    Q_OBJECT

  public:
    my_dialog_accept_file_path(QWidget* parent);
    ~my_dialog_accept_file_path();

    QPushButton*          btn_ok;
    my_line_edit_exe_path* my_lineEdit_exe_path_;    //自定的QLineEdit

  public:
  signals:
    void str_path_ok();

  protected:
    void closeEvent(QCloseEvent*) override;
};
