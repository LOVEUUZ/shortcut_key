#pragma once

#include <QLineEdit>

class My_lineEdit_exe_path : public QLineEdit {
    Q_OBJECT

  public:
    My_lineEdit_exe_path(QWidget* parent = nullptr);
    ~My_lineEdit_exe_path();

    void dragEnterEvent(QDragEnterEvent* event) override; //拖放操作进入窗口部件的边界时被调用
    void dropEvent(QDropEvent* event) override;           //拖放操作完成，数据被放置在窗口部件上时被调用

  private:
};
