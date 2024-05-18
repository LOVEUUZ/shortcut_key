#pragma once

#include <QDialog>
#include <QPushButton>
#include "my_lineedit_exe_path.h"
// #include "SubStyledItemDelegate.h"


class My_dialog_accept_filePath : public QDialog {
    Q_OBJECT

  public:
    My_dialog_accept_filePath(QWidget* parent);
    ~My_dialog_accept_filePath();

    QPushButton*          btn_ok;
    My_lineEdit_exe_path* my_lineEdit_exe_path_;

  public:
  signals:
    void str_path_ok(const QString& text);

};
