#pragma once

#include <QLineEdit>
#include <QFileDialog>

//自定的QLineEdit，拥有拖拽文件或文件夹，无法选中编辑的功能
class my_line_edit_exe_path final : public QLineEdit {
    Q_OBJECT

  public:
    my_line_edit_exe_path(QWidget* parent = nullptr);
    ~my_line_edit_exe_path();

    void dragEnterEvent(QDragEnterEvent* event) override; //拖放操作进入窗口部件的边界时被调用
    void dropEvent(QDropEvent* event) override;           //拖放操作完成，数据被放置在窗口部件上时被调用

    //处理双击事件
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
};
