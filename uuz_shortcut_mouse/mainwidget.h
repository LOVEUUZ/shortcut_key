#pragma once

#include <iostream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QtWidgets/QWidget>
#include <QStackedWidget>
#include <QTimer>
#include <QFont>
#include <QSystemTrayIcon>


#include "ui_mainwidget.h"
#include "iconButton.h"
#include "icons_inner_widget.h"
#include "search_content.h"
#include "Search_line.h"
#include "WindowsHookKeyEx.h"
#include "WindowsHookMouseEx.h"
#include "keyEvent.h"

#include "json.hpp"


class MainWidget : public QWidget {
    Q_OBJECT

    //仅用于子窗口调用焦点转移信号
    friend Search_line;
    friend icon_button;
    friend Search_content;

  public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget() override;

    static MainWidget* get_mainWidget() { return main_widget_; }

  private:
    // Ui::MainWidgetClass ui;
    static MainWidget* main_widget_;

    //主要模块  
    Search_line*        search_line;         //everything用搜索框
    QStackedWidget*     stacked_widget;      //用来包装下面两个窗口
    Icons_inner_widget* icons_inner_widget;  //图标显示窗口
    Search_content*     search_inner_widget; //搜索内容窗口

    //搜索栏相关功能
    void    init_search_line();
    QTimer* searchTimer; //避免文本框 textChanged 触发过快


    //布局相关
    void         init_layout(); //初始化布局
    QVBoxLayout* topLayout;
    QVBoxLayout* v_search_and_grid;
    QVBoxLayout* top_layout;
    QVBoxLayout* iconsLayout; //下半部分用来为icons_inner_widget设置内边距的布局

    //快捷呼出相关，暂定为连续按下两次ctrl
    void                init_shortcutKey();
    QTimer*             ctrlPressTimer;   //定时
    int                 ctrlPressCount;   //计数
    WindowsHookKeyEx*   windowsKeyHookEx; //windows键盘钩子
    WindowsHookMouseEx* windowsMouseHook; //windows鼠标钩子
    void                setKeyEvent();    //注册键盘事件
    void                setMouseEvent();  //注册鼠标事件

    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

    //配置相关，目前主要是窗口坐标的记录
    void              moveEvent(QMoveEvent* event) override; //用于退出程序的时候进保存进配置文件，方便下一次启动的时候也是在该位置开启
    void              init_coordinate();
    QString           file_path;
    QFile*            file_config;
    nlohmann::json    json_config;
    QMap<int, QPoint> screens_coordinate; //多屏幕坐标记录，和json冗余，但用起来方便
  public:
    nlohmann::json& get_jsonConfig() { return json_config; }

  private:
    //重定义关闭按钮，改为隐藏
    void closeEvent(QCloseEvent* event) override;

    //系统托盘
    void            init_tray();
    QMenu           trayMenu; //托盘菜单
    QSystemTrayIcon trayIcon; //托盘图标


  signals:
    void sig_moveFocus(QWidget*); //焦点移动

    void sig_modifyConfig();

  private slots:
    void slot_showStackedWidgetIndex(int index) const;
    void slot_moveFocus(QWidget*); //焦点移动
    void slot_onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

  public slots:
    void slot_modifyConfig();
};
