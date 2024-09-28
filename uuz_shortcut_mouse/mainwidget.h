#pragma once

#include <iostream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QtWidgets/QWidget>

#include "ui_mainwidget.h"
#include "iconButton.h"
#include "icons_inner_widget.h"

class MainWidget : public QWidget {
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

    QVBoxLayout*        topLayout;
    QWidget*            icons_widget;
    Icons_inner_widget* icons_inner_widget;

    QLineEdit* search_line;

    //�������
    QVBoxLayout* v_search_and_grid;
    QVBoxLayout* top_layout;
    QVBoxLayout* iconsLayout; //�°벿������Ϊicons_inner_widget�����ڱ߾�Ĳ���

    // void paintEvent(QPaintEvent* event);
    void                   init_layout();       //��ʼ������


};
