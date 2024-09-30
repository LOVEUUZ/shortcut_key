#pragma once

#include <iostream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QtWidgets/QWidget>
#include <QStackedWidget>

#include "ui_mainwidget.h"
#include "iconButton.h"
#include "icons_inner_widget.h"
#include "search_content.h"


class MainWidget : public QWidget {
    Q_OBJECT

  public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

  private:
    // Ui::MainWidgetClass ui;

    //主要模块  
    QLineEdit*          search_line;  //everything用搜索框
    QStackedWidget*     stacked_widget;      //用来包装下面两个窗口
    Icons_inner_widget* icons_inner_widget;  //图标显示窗口
    Search_content*     search_inner_widget; //搜索内容窗口

    void init_search_line();

    //布局相关
    QVBoxLayout* topLayout;
    QVBoxLayout* v_search_and_grid;
    QVBoxLayout* top_layout;
    QVBoxLayout* iconsLayout; //下半部分用来为icons_inner_widget设置内边距的布局

    // void paintEvent(QPaintEvent* event);
    void init_layout(); //初始化布局

  private slots:
      void slot_show_stackedWidget_index(const int index) const;

};
