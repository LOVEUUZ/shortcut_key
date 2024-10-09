#pragma once

#include <iostream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QtWidgets/QWidget>
#include <QStackedWidget>
#include <QTimer>
#include <QFont>

#include "ui_mainwidget.h"
#include "iconButton.h"
#include "icons_inner_widget.h"
#include "search_content.h"
#include "Search_line.h"
#include "WindowsHookEx.h"
#include "keyEvent.h"


class MainWidget : public QWidget {
	Q_OBJECT

		//仅用于子窗口调用焦点转移信号
		friend Search_line;
	friend IconButton;
	friend Search_content;

public:
	MainWidget(QWidget* parent = nullptr);
	~MainWidget() override;

private:
	// Ui::MainWidgetClass ui;

	//主要模块  
	Search_line* search_line;         //everything用搜索框
	QStackedWidget* stacked_widget;      //用来包装下面两个窗口
	Icons_inner_widget* icons_inner_widget;  //图标显示窗口
	Search_content* search_inner_widget; //搜索内容窗口

	//搜索栏相关功能
	void    init_search_line();
	QTimer* searchTimer; //避免文本框 textChanged 触发过快


	//布局相关
	QVBoxLayout* topLayout;
	QVBoxLayout* v_search_and_grid;
	QVBoxLayout* top_layout;
	QVBoxLayout* iconsLayout; //下半部分用来为icons_inner_widget设置内边距的布局

	// void paintEvent(QPaintEvent* event);
	void init_layout(); //初始化布局

	//隐藏与显示该窗口快捷键，暂定为连续按下两次ctrl
	void init_shortcut_key();
	QTimer* ctrlPressTimer;		  //定时
	int ctrlPressCount;				  //计数
	WindowsHookEx* windowsHookEx;//windows钩子
	void setKeyEvent();				  //注册事件

signals:
	void sig_move_focus(QWidget*); //焦点移动


private slots:
	void slot_show_stackedWidget_index(int index) const;
	void slot_move_focus(QWidget*); //焦点移动
};
