#pragma once

#include <iostream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QtWidgets/QWidget>

#include "ui_mainwidget.h"
#include "iconButton.h"

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget* parent = nullptr);
	~MainWidget();

  void gets() {
	  std::cout << icons_inner_widget->pos().x() << "\n";
	  std::cout << icons_inner_widget->pos().y() << "\n";
  }

private:
	// Ui::MainWidgetClass ui;


	static constexpr int x = 8;
	static constexpr int y = 4;
	static constexpr int icon_button_size = 95;
	static constexpr int sum = x * y;

	QPushButton* q_push_button_arr[x][y];
	QVector <QPair<int, int>> vec_coordinate;  //icon_button的左上角坐标
	QRect first_icon_coordinate;

	QVBoxLayout* topLayout;
	QWidget* icons_widget;
	QWidget* icons_inner_widget;
	
	QLineEdit* search_line;

	//布局相关
	QVBoxLayout* v_search_and_grid;
	QVBoxLayout* top_layout;
	QVBoxLayout* iconsLayout;			  //下半部分用来为icons_inner_widget设置内边距的布局

	// void paintEvent(QPaintEvent* event);
	void init_layout();							//初始化布局
	void init_coordinateut();			  //初始化坐标
	void init_rendering();			    //检测坐标以及渲染范围
};
