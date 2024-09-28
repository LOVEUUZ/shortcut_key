#include "mainwidget.h"

#include <iostream>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>

MainWidget::MainWidget(QWidget* parent)
	: QWidget(parent)
{
	// ui.setupUi(this);
	setObjectName("mainWidget");
	// setWindowFlags(Qt::FramelessWindowHint);
	init_layout();



	setStyleSheet("MainWidget{background-color: #b7b7b7;} ");


	

}

MainWidget::~MainWidget()
{

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
	topLayout->setSpacing(5); // 设置上部布局内部组件间距为5

	search_line = new QLineEdit(this);
	search_line->setFixedSize(800, 60); // 设置上部 QLineEdit 的固定高度为60
	search_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 设置为可扩展，宽度可以填满

	topLayout->addWidget(search_line);

	// 3. 创建下部栅格布局，并设置边距和间距
	icons_widget = new QWidget(this);
	icons_widget->setFixedSize(800, 400);

	iconsLayout = new QVBoxLayout(icons_widget);
	iconsLayout->setContentsMargins(10, 5, 5, 10); // 设置内边距
	iconsLayout->setSpacing(0); // 设置间距为0

	// 4. 创建内部 widget 并设置大小和颜色
	icons_inner_widget = new Icons_inner_widget(icons_widget);
	// icons_inner_widget->setStyleSheet("background-color: #fff111;"); // 设置背景颜色
	icons_inner_widget->setContentsMargins(5, 5, 5, 5);

	iconsLayout->addWidget(icons_inner_widget); // 添加到布局中

	// 5. 添加上部和下部布局到主布局
	v_search_and_grid->addLayout(topLayout); // 将上部布局添加到主布局
	v_search_and_grid->addWidget(icons_widget); // 将下部widget添加到主布局

	// 自动调整窗口大小
	adjustSize(); // 调整窗口大小以适应内容
	setFixedSize(size()); // 将当前大小设置为固定大小
}

