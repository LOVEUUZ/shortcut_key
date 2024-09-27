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
	QVector <QPair<int, int>> vec_coordinate;  //icon_button�����Ͻ�����
	QRect first_icon_coordinate;

	QVBoxLayout* topLayout;
	QWidget* icons_widget;
	QWidget* icons_inner_widget;
	
	QLineEdit* search_line;

	//�������
	QVBoxLayout* v_search_and_grid;
	QVBoxLayout* top_layout;
	QVBoxLayout* iconsLayout;			  //�°벿������Ϊicons_inner_widget�����ڱ߾�Ĳ���

	// void paintEvent(QPaintEvent* event);
	void init_layout();							//��ʼ������
	void init_coordinateut();			  //��ʼ������
	void init_rendering();			    //��������Լ���Ⱦ��Χ
};
