#include "mainwidget.h"

#include <iostream>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>

MainWidget::MainWidget(QWidget* parent) : QWidget(parent) {
  // ui.setupUi(this);
  setObjectName("mainWidget");
  // setWindowFlags(Qt::FramelessWindowHint);

  init_layout();

  init_search_line();

  // setStyleSheet("MainWidget{background-color: #b7b7b7;} ");
}

MainWidget::~MainWidget() {}


void MainWidget::init_search_line() {
  //用于避免定时器
  searchTimer = new QTimer(this);
  searchTimer->setSingleShot(true);             // 定时器只执行一次

  //当定时器倒计时结束后触发搜索任务
  connect(searchTimer, &QTimer::timeout, this, [this]() {
    // slot_show_stackedWidget_index(text.isEmpty() ? 0 : 1);
    emit search_inner_widget->slot_text_change(search_line->text());       // 发出信号进行搜索
  });

  //当搜索栏文本改变后触发定时器倒计时一次
  connect(search_line, &QLineEdit::textChanged, this, [this]() {
    searchTimer->start(300); // 启动定时器，300毫秒后触发
  });

  //控制 stacked_widget 显示为 search_inner_widget
  connect(search_line, &QLineEdit::textChanged, this, [this]() {
    QString text = this->search_line->text();

    int index = 0;
    if (text.isEmpty()) index = 0; // 文本为空，显示  icons_inner_widget 图标显示窗口
    else index                = 1; // 显示为 search_inner_widget 搜索内容窗口

    slot_show_stackedWidget_index(index);
  });
}


// 设置布局
void MainWidget::init_layout() {
  // 1. 创建主布局并设置无边距
  v_search_and_grid = new QVBoxLayout(this);
  v_search_and_grid->setContentsMargins(0, 0, 0, 0); // 设置主布局无边距
  setLayout(v_search_and_grid);

  // 2. 创建上部垂直布局，并设置边距和间距
  topLayout = new QVBoxLayout();
  topLayout->setContentsMargins(5, 5, 5, 0); // 设置上部布局与四周的距离
  topLayout->setSpacing(5);                  // 设置上部布局内部组件间距为5

  search_line = new QLineEdit(this);
  search_line->setFixedSize(800, 60);                                     // 设置上部 QLineEdit 的固定高度为60
  search_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 设置为可扩展，宽度可以填满

  topLayout->addWidget(search_line);

  // 3. 下半部分布局，并设置边距和间距
  stacked_widget = new QStackedWidget(this);
  stacked_widget->setFixedSize(800, 400);
  stacked_widget->setContentsMargins(10, 5, 5, 10); // 设置内边距

  iconsLayout = new QVBoxLayout(stacked_widget);
  iconsLayout->setContentsMargins(10, 5, 5, 10); // 设置内边距
  iconsLayout->setSpacing(0);                    // 设置间距为0

  // 4. 创建内部 widget 并设置大小和颜色,添加进入栈窗口显示
  icons_inner_widget = new Icons_inner_widget(stacked_widget);
  // icons_inner_widget->setStyleSheet("background-color: #123456;"); // 设置背景颜色
  // icons_inner_widget->setContentsMargins(5, 5, 5, 5);
  // iconsLayout->addWidget(icons_inner_widget); // 添加到布局中

  search_inner_widget = new Search_content(stacked_widget);
  // search_inner_widget->setStyleSheet("background-color: red;"); // 设置背景颜色
  // search_inner_widget->setContentsMargins(5, 5, 5, 5);
  // iconsLayout->addWidget(search_inner_widget); // 添加到布局中

  stacked_widget->addWidget(icons_inner_widget);
  stacked_widget->addWidget(search_inner_widget);


  // 5. 添加上部和下部布局到主布局
  v_search_and_grid->addLayout(topLayout);      // 将上部布局添加到主布局
  v_search_and_grid->addWidget(stacked_widget); // 将下部widget添加到主布局

  // 自动调整窗口大小
  adjustSize();         // 调整窗口大小以适应内容
  setFixedSize(size()); // 将当前大小设置为固定大小
}

void MainWidget::slot_show_stackedWidget_index(const int index) const {
  stacked_widget->setCurrentIndex(index);
}
