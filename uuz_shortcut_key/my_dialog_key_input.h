#pragma once

#include <set>
#include <sstream>
#include <iostream>

#include <QLineEdit>
#include <QDialog>
#include <QHBoxLayout>

#include "mainwindow.h"
#include "keyEvent.h"
#include "startQuickly.h"

//用于获取快捷键输入的dialog
class my_dialog_key_input : public QDialog {
    Q_OBJECT

  public:
    my_dialog_key_input(QWidget* parent = nullptr);
    ~my_dialog_key_input();

    QLineEdit*        line_edit;
    QPushButton*      btn_reset;
    QPushButton*      btn_ok;
    QHBoxLayout*      layout;
    QStringList       tmp_list;
    std::stringstream str_key_tmp; //存储当前QLineEdit显示的内容
    ShortcutKeyMsg    shortcut_key_msg_;

  private:
    StartQuickly*  start_quickly_;
    WindowsHookKeyEx* ptr_windows_hook;

    std::set<uint64_t> set_; //确保一个按键只能绑定一次，重置按钮按下的时候清除这里


    void setFunc(); //策略模式，修改键盘钩子的逻辑为此处所用
    void set_key_event(const KeyEvent & key_event);

    void initView();
    void initConnect();

  protected:
    void closeEvent(QCloseEvent* event) override;

  signals:
    void editingCompleted(const std::vector<std::string>);
};
